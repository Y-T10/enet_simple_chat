#include"config.hpp"
#include"enet_host.hpp"
#include"sig_event.hpp"
#include"enet_packet_stream.hpp"
#include<algorithm>
#include<memory>
#include<thread>
#include<boost/unordered_map.hpp>
#include<iostream>
#include<msgpack.hpp>

using namespace std;
using namespace msgpack;

using ClientID = uint64_t;
using ENetPeerID = ENetPeer*;

struct user_info {
    ClientID m_id;
    string m_name;
};

class server_system : private boost::noncopyable {
    public:
    server_system() noexcept
    :m_net(nullptr)
    ,m_isQuit(false){
    }

    ~server_system(){
        m_net = nullptr;
    }

    void init() noexcept{
        m_net = std::make_unique<NetHost>();
        ENetAddress addr = {.host = ENET_HOST_ANY, .port = PORT};
        m_net->set_host({128, 2, 0, 0, &addr});
        m_sig = std::make_unique<SigEvent>();
        m_sig->set_signal({SIGHUP, SIGTERM});
        m_user.clear();
    };

    void update() noexcept{
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

    const bool isQuit() noexcept{
        return m_isQuit;
    }

    private:
    void on_connect(const ENetEvent* e){
        e->peer->data = NULL;
        m_user[e->peer] = {};
        std::cerr << "[connection]" << std::endl;
    }
    void on_recv(const ENetEvent* e){
        if(e->peer->data == NULL){
            e->peer->data = (void*)((uintptr_t)(0xff));
            PacketUnpacker unpacker(e->packet);
            m_user[e->peer] = {
                .m_id = unpacker.read<ClientID>(),
                .m_name = unpacker.read<std::string>()
            };
            const std::string user_message = m_user[e->peer].m_name +" has entered.";
            PacketStream pstream;
            pstream.write(user_message);
            m_net->broadcast(1, pstream.packet());
            m_net->flush();
            std::cerr << "[new user] "
                      << "name: " << m_user[e->peer].m_name << ", "
                      << "id: "   << m_user[e->peer].m_id   << std::endl;
            return;
        }
        object_handle result = unpack((const char*)(e->packet->data), e->packet->dataLength);
        const auto user_info = m_user[e->peer];
        const auto msg = result.get().as<std::string>();
        const std::string user_message = user_info.m_name + ": "+ msg;
        PacketStream pstream;
        pstream.write(user_message);
        m_net->broadcast(1, pstream.packet());
        m_net->flush();
    }
    void on_disconnect(const ENetEvent* e){
        const auto usr_info = m_user[e->peer];
        const std::string disco_message = usr_info.m_name + " has disconnected.\n";
        m_user.erase(e->peer);
        e->peer->data = NULL;
        PacketStream pstream;
        pstream.write(disco_message);
        m_net->broadcast(1, pstream.packet());
        m_net->flush();
        enet_peer_disconnect(e->peer, 0);
        std::cerr   << "[disconnect]"
                    << " user: " << usr_info.m_name
                    << " id: " << usr_info.m_id
                    << std::endl;
    }

    void on_signal(const int sig){
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

    private:
    std::unique_ptr<NetHost> m_net;
    std::unique_ptr<SigEvent> m_sig;
    boost::unordered_map<ENetPeerID, user_info> m_user;
    bool m_isQuit;
};

int  main(int argc, char ** argv) {
    if(enet_initialize() != 0) {
        fprintf(stderr, "Could not initialize enet.\n");
        return 0;
    }

    auto m_server_sys = std::make_unique<server_system>();
    m_server_sys->init();
    while(!m_server_sys->isQuit()){
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        m_server_sys->update();
    }
    m_server_sys = nullptr;

    enet_deinitialize();
}