#include"client.hpp"

#include<cassert>
#include<iostream>
#include<cstdint>
#include"enet_utils.hpp"
#include"enet_packet_stream.hpp"
#include"packet_fmt.hpp"
#include<functional>
#include<thread>
#include<msgpack.hpp>

chat_system::chat_system() noexcept
:m_io(nullptr)
,m_net(nullptr)
,m_quit_flag(false)
,m_info()
,m_ch_max(2){
}

chat_system::~chat_system(){
    m_io = nullptr;
    m_net = nullptr;
}

const bool chat_system::initilize(){
    m_io = std::make_unique<console_io>();
    m_net = std::make_unique<NetClient>();
    return m_io && m_net && m_net->set_host({
        .peer_max = 1, .packet_ch_max= m_ch_max,
        .down_size = 0, .up_size = 0,
        .address = NULL
    });
}

void chat_system::register_user_info(const UserInfo& info){
    m_info = info;
    fprintf(stderr, "user name: %s %lu\n", m_info.name.c_str(), m_info.id);
}

const bool chat_system::connect_server(const std::string& hostname, const uint32_t port){
    if((bool)m_net == false || (*m_net) == false){
        return false;
    }
    return m_net->request_connection(CreateENetAddress(hostname, port), m_ch_max);
}

const bool chat_system::is_quit() noexcept{
    return m_quit_flag;
}

void chat_system::update() noexcept{
    using namespace std::placeholders;
    //文字列入力を処理する
    m_io->handle_input(std::bind(&chat_system::on_io_input, this, _1));
    //ネットワークのイベントを処理する
    m_net->handle_host_event(std::bind(&chat_system::on_net, this, _1));
}

void chat_system::on_io_input(const std::string& msg){
    if(msg == "q" && !m_quit_flag){
        m_net->close_all();
        return;
    }
    m_net->handle_peer(
        [](const ENetPeer* p)->const bool{return (uintptr_t)(p->data) == (uintptr_t)(0xff);},
        [this,&msg](ENetPeer* p){
            PacketStream pstream;
            pstream.write(this->m_info.id);
            pstream.write(msg);
            enet_peer_send(p, 1, pstream.packet());
        });
    m_net->flush();
    return;
}

void chat_system::on_net(const ENetEvent* e){
    if(e->type == ENET_EVENT_TYPE_NONE){
        return;
    }
    if(e->type == ENET_EVENT_TYPE_CONNECT){
        e->peer->data = (void*)(uintptr_t)(0xff);
        PacketStream pstream;
        pstream.write(SysRequestID::REGISTER_USER);
        pstream.write(m_info);
        enet_peer_send(e->peer, 0, pstream.packet());
        m_net->flush();
        return;
    }
    if(e->type == ENET_EVENT_TYPE_RECEIVE){
        using namespace msgpack;
        object_handle result = unpack((const char*)e->packet->data, e->packet->dataLength);
        m_io->add_message("Message: " + result->as<std::string>());
        return;
    }
    if(e->type == ENET_EVENT_TYPE_DISCONNECT){
        if(e->peer->data != NULL){
            m_io->add_message("disconnected.");
        }else{
            m_io->add_message("connection failed.");
        }
        e->peer->data = NULL;
        m_quit_flag = true;
        return;
    }
};

int main(int argc, char ** argv) {
    if (argc < 5) {
        printf("Usage: [user name] [user ID] [server host] [server port]\n");
        exit(1);
    }

    if (enet_initialize() != 0) {
        printf("Could not initialize enet.\n");
        return 0;
    }

    //チャットシステムを作成する
    auto chat_sys = std::make_unique<chat_system>();
    if(!chat_sys->initilize()){
        chat_sys = nullptr;
        enet_deinitialize();
        return 1;
    }
    //ユーザ情報を登録
    chat_sys->register_user_info({.name = argv[1], .id = (ClientID)std::atol(argv[2])});
    //サーバに接続要求を投げる
    if(!chat_sys->connect_server(argv[3], std::atoi(argv[4]))){
        chat_sys = nullptr;
        enet_deinitialize();
        return 1;
    }

    while(chat_sys->is_quit() == false){
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        chat_sys->update();
    }

    chat_sys = nullptr;   
    enet_deinitialize();
    return 0;
}