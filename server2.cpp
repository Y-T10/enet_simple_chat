#include"config.hpp"
#include"net_host.hpp"
#include"SigEvent.hpp"
#include<algorithm>
#include<memory>
#include<optional>
#include<thread>
#include<boost/unordered_map.hpp>
#include<iostream>
#include<msgpack.hpp>

using namespace std;
using namespace msgpack;

using ClientID = uintptr_t;

struct user_info {
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
        m_net->set_host({128, 2, 0, 0, {.host = ENET_HOST_ANY, .port = PORT}});
        m_sig = std::make_unique<SigEvent>();
        m_sig->set_signal({SIGHUP, SIGTERM});
        m_user.clear();
    };

    void update() noexcept{
        //シグナルを調べる
        using namespace std::placeholders;
        const auto sig_handler = std::bind(&server_system::on_singnal, this, _1);
        m_sig->handle_signal(sig_handler);
        //ネットワークのイベントを処理する
        const auto net_handler = std::bind(&server_system::on_net, this, _1);
        m_net->handle_host_event(net_handler);
    }

    const bool isQuit() noexcept{
        return m_isQuit;
    }

    private:
    void on_net(const ENetEvent* e){
        if(e->type == ENET_EVENT_TYPE_NONE){
            return;
        }
        const auto on_connect = [this](const ENetEvent* e){
            const ClientID new_id = e->data;
            e->peer->data = (void*)new_id;
            const user_info new_user = {.m_name = ""};
            m_user.emplace(new_id, new_user);
            std::cerr << "[connection]"
                      << " id: "   << new_id
                      << std::endl;
        };
        if(e->type == ENET_EVENT_TYPE_CONNECT){
            on_connect(e);
        }
        const ClientID id = (uintptr_t)(e->peer->data);
        const auto on_recv = [this,&id](const ENetEvent* e){
            const ClientID id = (uintptr_t)(e->peer->data);
            object_handle result = unpack((const char*)(e->packet->data), e->packet->dataLength);
            const auto msg = result.get().as<std::string>();
            if(m_user[id].m_name == ""){
                m_user[id].m_name = msg;
            }
            const std::string user_message = m_user[id].m_name + ": "+ msg +"\n";
            ENetPacket* packet = enet_packet_create(
                user_message.data(), user_message.size(),
                ENET_PACKET_FLAG_RELIABLE);
            m_net->broadcast(0, packet);
            m_net->flush();
        };
        if(e->type == ENET_EVENT_TYPE_RECEIVE){
            on_recv(e);
        }
        const auto on_disconnect = [this,&id](const ENetEvent* e){
            const std::string disco_message = m_user[id].m_name + " has disconnected.\n";
            std::cerr   << "[disconnect]"
                        << " user: " << m_user[id].m_name
                        << " id: " << id
                        << std::endl;
            ENetPacket* packet = enet_packet_create(
                    disco_message.data(), disco_message.size(),
                    ENET_PACKET_FLAG_RELIABLE);
            m_net->broadcast(0, packet);
            m_net->flush();
            m_user.erase(m_user.find(id));
        };
        if(e->type == ENET_EVENT_TYPE_DISCONNECT){
            on_disconnect(e);
        }
    };

    void on_singnal(const int sig){
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
    boost::unordered_map<ClientID, user_info> m_user;
    bool m_isQuit;
};

int  main(int argc, char ** argv) {
    if(enet_initialize() != 0) {
        fprintf(stderr, "Could not initialize enet.\n");
        return 0;
    }

    auto m_server_sys = std::make_unique<server_system>();
    while(!m_server_sys->isQuit()){
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        m_server_sys->update();
    }
    m_server_sys = nullptr;

    enet_deinitialize();
}