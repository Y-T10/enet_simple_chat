#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<enet/enet.h>
#include"config.hpp"
#include"enet_send.hpp"
#include"meditator.hpp"
#include"net_host.hpp"
#include<algorithm>
#include<memory>
#include<optional>
#include<thread>
#include<boost/unordered_map.hpp>
#include<iostream>
#include<msgpack.hpp>
#include<csignal>
#include<cerrno>
#include<initializer_list>
#include<signal.h>

using namespace std;
using namespace msgpack;

enum NetEvent : uint8_t {
    NONE = 0, 
    CONNECT,
    DISCONNECT,
    RECEIVE,
    REFUSE,         ///接続拒否
};

using ClientID = uintptr_t;

struct user_info {
    string m_name;
};

class chat_user_namager : public Colleague {
    public:
    chat_user_namager() noexcept
    :m_user_list(){};
    ~chat_user_namager() noexcept{
    }

    const bool add_new_user(const ClientID user_id, const user_info& info) noexcept{
        if(m_user_list.contains(user_id)){
            return false;
        }
        m_user_list[user_id] = info;
        return true;
    }

    const std::optional<user_info> get_user_info(const ClientID user_id) noexcept{
        if(!m_user_list.contains(user_id)){
            return nullopt;
        }
        return std::optional<user_info>(m_user_list[user_id]);
    };

    void replace_user_info(const ClientID user_id, const user_info& info) noexcept{
        if(!m_user_list.contains(user_id)){
            return;
        }
        m_user_list[user_id] = info;
    };

    void remove_user_info(const ClientID user_id) noexcept{
        if(!m_user_list.contains(user_id)){
            return;
        }
        m_user_list.extract(m_user_list.find(user_id));
    };

    const size_t user_num() noexcept{
        return m_user_list.size();
    }

    private:
    boost::unordered_map<ClientID, user_info> m_user_list;
};

class SigEvent : public Colleague {
    public:

    SigEvent() noexcept
    :m_lastSignalID(0)
    ,m_sigMask(){};

    ~SigEvent() = default;

    const bool set_signal(const std::vector<int>& sigs){
        //シグナルのマスクを初期化する
        sigemptyset(&m_sigMask);
        //シグナルの設定
        for(auto &&iSig: sigs){
            if(iSig == SIGKILL || iSig == SIGSTOP){
                continue;
            }
            //シグナル番号は1以上が有効で0以下は無効
            assert(iSig > 0);
            //シグナルマスクを設定する
            sigaddset(&m_sigMask, iSig);
        }
        //シグナルハンドラーを無効にする
        //sigtimedwaitでシグナルを受信するには、
        //指定のシグナルをシグナルハンドラーに送らないようにする．
        sigprocmask(SIG_BLOCK, &m_sigMask, NULL);
    }

    void handle_signal(const std::function<void(const int)>& handler){
        //すぐ返す
        constexpr struct timespec wait_time = { .tv_sec = 0, .tv_nsec = 0 };
        //シグナルを取り出す
        const int signal = sigtimedwait(&m_sigMask, NULL, &wait_time);
        //監視対象のシグナルがあるか
        if(signal > 0){
            handler(signal);
            return;
        }
    }

    private:
    int m_lastSignalID;
    sigset_t m_sigMask;
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
        m_user = std::make_unique<chat_user_namager>();
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
    const bool on_net(const ENetEvent* e){
        const auto on_connect = [this](const ENetEvent* e) -> const bool{
            const ClientID new_id = (uintptr_t)(e->data);
            e->peer->data = (void*)new_id;
            m_user->add_new_user(new_id, {.m_name = ""});
            return true;
        };
        const auto on_recv = [this](const ENetEvent* e, const ClientID id) -> const bool{
            const ClientID id = (uintptr_t)(e->peer->data);
            const auto info = m_user->get_user_info(id);
            assert(info.has_value());
            object_handle result = unpack((const char*)(e->packet->data), e->packet->dataLength);
            const auto msg = result.get().as<std::string>();
            if(info->m_name == ""){
                const user_info new_user = {
                    .m_name = msg
                };
                m_user->replace_user_info(id, new_user);
                const std::string join_message = new_user.m_name + " has connected\n";
                ENetPacket* packet = enet_packet_create(
                    join_message.data(), join_message.size(),
                    ENET_PACKET_FLAG_RELIABLE);
                m_net->broadcast(0, packet);
                m_net->flush();
                std::cerr   << "[connection]"
                            << " user: " << new_user.m_name
                            << " id: " << id
                            << std::endl;
                return true;
            }
            const std::string user_message = info->m_name + ": "+ msg +"\n";
            ENetPacket* packet = enet_packet_create(
                    user_message.data(), user_message.size(),
                    ENET_PACKET_FLAG_RELIABLE);
            m_net->broadcast(0, packet);
            m_net->flush();
            std::cerr   << "[message]"
                << " user: " << info->m_name
                << " id: " << id
                << " message: " << msg
                << std::endl;
            return true;
        };
        const auto on_disconnect = [this](const ENetEvent* e, const ClientID id) -> const bool{
            const auto info = m_user->get_user_info(id);
            m_user->remove_user_info(id);
            assert(info.has_value());
            const std::string disco_message = info->m_name + " has disconnected.\n";
            ENetPacket* packet = enet_packet_create(
                    disco_message.data(), disco_message.size(),
                    ENET_PACKET_FLAG_RELIABLE);
            m_net->broadcast(0, packet);
            m_net->flush();
            std::cerr   << "[disconnect]"
                        << " user: " << info->m_name
                        << " id: " << id
                        << std::endl;
            return true;
        };
        if(e->type == ENET_EVENT_TYPE_CONNECT){
            return on_connect(e);
        }
        const ClientID id = (uintptr_t)(e->peer->data);
        if(e->type == ENET_EVENT_TYPE_RECEIVE){
            return on_recv(e, id);
        }
        if(e->type == ENET_EVENT_TYPE_DISCONNECT){
            return on_disconnect(e, id);
        }
        return false;
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
    std::unique_ptr<chat_user_namager> m_user;
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