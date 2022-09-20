#include"client.hpp"

#include<cassert>
#include<iostream>
#include<cstdint>
#include"config.hpp"
#include"enet_utils.hpp"
#include"enet_packet_stream.hpp"
#include<functional>
#include<thread>
#include<msgpack.hpp>

chat_system::chat_system() noexcept
:m_io(nullptr)
,m_net(nullptr)
,m_quit_flag(false)
,m_info(){
}

chat_system::~chat_system(){
    m_io = nullptr;
    m_net = nullptr;
}

const bool chat_system::initilize(const std::string& name, const std::string& id){
    m_info.name = name;
    m_info.id = std::atol(id.c_str());
    fprintf(stderr, "user name: %s %lu\n", m_info.name.c_str(), m_info.id);
    m_io = std::make_unique<console_io>();
    m_net = std::make_unique<NetClient>();
    m_net->set_host({1, 2, 0, 0, NULL});
    return m_net->request_connection(CreateENetAddress(HOST, PORT), 2, 0);
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
            pstream.write(msg);
            enet_peer_send(p, 0, pstream.packet());
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
    if (argc < 3) {
        printf("Usage: client name ID\n");
        exit(1);
    }

    if (enet_initialize() != 0) {
        printf("Could not initialize enet.\n");
        return 0;
    }

    //チャットシステムを作成する
    auto chat_sys = std::make_unique<chat_system>();
    if(!chat_sys->initilize(argv[1], argv[2])){
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