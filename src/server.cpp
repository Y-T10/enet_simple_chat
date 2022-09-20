#include"server.hpp"

#include"enet_packet_stream.hpp"
#include<algorithm>
#include<thread>
#include<iostream>
#include<msgpack.hpp>

using namespace std;
using namespace msgpack;

server_system::server_system() noexcept
:m_net(nullptr)
,m_sig(nullptr)
,m_user()
,m_isQuit(false){
}

server_system::~server_system(){
    m_net = nullptr;
    m_sig = nullptr;
    m_user.clear();
    m_isQuit = false;
}

void server_system::init() noexcept{
    m_net = std::make_unique<NetHost>();
    m_sig = std::make_unique<SigEvent>();
    m_user.clear();
};

const bool server_system::setup_server(const enet_uint16 port) noexcept{
    ENetAddress addr = {.host = ENET_HOST_ANY, .port = port};
    if(!m_sig->set_signal({SIGHUP, SIGTERM})){
        return false;
    }
    return m_net->set_host({
        .peer_max = 128, .packet_ch_max= 2,
        .down_size = 0, .up_size = 0,
        .address = &addr
    });
}

void server_system::update() noexcept{
    //シグナルを調べる
    using namespace std::placeholders;
    const auto sig_handler = std::bind(&server_system::on_signal, this, _1);
    m_sig->handle_signal(sig_handler);
    //ネットワークのイベントを処理する
    m_net->handle_host_event(
        std::bind(&server_system::on_disconnect, this, _1),
        [](const ENetEvent* e){},
        std::bind(&server_system::on_connect, this, _1),
        std::bind(&server_system::on_recv, this, _1)
    );
}

const bool server_system::isQuit() noexcept{
    return m_isQuit;
}

void server_system::on_connect(const ENetEvent* e){
    e->peer->data = NULL;
    m_user[e->peer] = {};
    std::cerr << "[connection]" << std::endl;
}

void server_system::on_recv(const ENetEvent* e){
    if(e->peer->data == NULL){
        e->peer->data = (void*)((uintptr_t)(0xff));
        PacketUnpacker unpacker(e->packet);
        m_user[e->peer] = unpacker.read<UserInfo>();
        const std::string user_message = m_user[e->peer].name +" has entered.";
        PacketStream pstream;
        pstream.write(user_message);
        m_net->broadcast(1, pstream.packet());
        m_net->flush();
        std::cerr << "[new user] "
                  << "name: " << m_user[e->peer].name << ", "
                  << "id: "   << m_user[e->peer].id   << std::endl;
        return;
    }
    object_handle result = unpack((const char*)(e->packet->data), e->packet->dataLength);
    const auto user_info = m_user[e->peer];
    const auto msg = result.get().as<std::string>();
    const std::string user_message = user_info.name + ": "+ msg;
    PacketStream pstream;
    pstream.write(user_message);
    m_net->broadcast(1, pstream.packet());
    m_net->flush();
}

void server_system::on_disconnect(const ENetEvent* e){
    const auto usr_info = m_user[e->peer];
    const std::string disco_message = usr_info.name + " has disconnected.\n";
    m_user.erase(e->peer);
    e->peer->data = NULL;
    PacketStream pstream;
    pstream.write(disco_message);
    m_net->broadcast(1, pstream.packet());
    m_net->flush();
    enet_peer_disconnect(e->peer, 0);
    std::cerr   << "[disconnect]"
                << " user: " << usr_info.name
                << " id: " << usr_info.id
                << std::endl;
}

void server_system::on_signal(const int sig){
    std::cerr << "[signal] id: " << sig << std::endl;
    //終了シグナルを受信したか
    if(sig == SIGTERM){
        //プログラムを正しく終了する(ex. プログラム終了メッセージを発行する)
        m_net->close_all();
        m_isQuit = true;
        return;
    }
    return;
};

int main(int argc, char ** argv) {
    if(argc < 2){
        printf("usage: server [port number]\n");
        return 1;
    }
    if(enet_initialize() != 0) {
        fprintf(stderr, "Could not initialize enet.\n");
        return 0;
    }

    auto m_server_sys = std::make_unique<server_system>();
    m_server_sys->init();
    if(!m_server_sys->setup_server(atoi(argv[1]))){
        m_server_sys = nullptr;
        enet_deinitialize();
        return 1;
    }
    while(!m_server_sys->isQuit()){
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        m_server_sys->update();
    }
    m_server_sys = nullptr;

    enet_deinitialize();

    return 0;
}