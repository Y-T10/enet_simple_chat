#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <enet/enet.h>
#include "config.hpp"
#include "enet_send.hpp"
#include "meditator.hpp"
#include<algorithm>
#include<memory>
#include<optional>
#include<list>
#include<thread>
#include <boost/unordered_map.hpp>

using namespace std;

void PrintPacket(const ENetPacket* packet){
    fprintf(stderr, "[Packet] ");
    fprintf(stderr, "data: ");
    for(size_t i = 0; i < packet->dataLength; ++i){
        fprintf(stderr, "%x ", packet->data[i]);
    }
    fprintf(stderr, "\n");
}

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

class chat_net : public Colleague {
    const bool is_there_peer(const ENetPeer* target) noexcept{
        for(size_t i = 0; i <m_server->peerCount; ++i){
            if(m_server->peers + i == target){
                return true;
            }
        }
        return false;
    }
    public:

    explicit chat_net(const enet_uint16 port, const size_t peer_max, const size_t ch) noexcept
    :m_server(NULL)
    ,m_last_recived(0)
    ,m_last_event(NetEvent::NONE)
    ,m_last_from(0){
        const ENetAddress address = {.host = ENET_HOST_ANY, .port = port};
        m_server = enet_host_create(&address, peer_max, ch, 0, 0);
    }

    ~chat_net(){
        if(m_server != NULL){
            enet_host_destroy(m_server);
        }
    }

    const bool is_valied() noexcept{
        return m_server != NULL;
    }

    void update(){
        assert(is_valied());
        ///イベントを受け取る
        ENetEvent event;
        //エラーが生じるまでイベントを処理する
        while(enet_host_service(m_server, &event, 0) >= 0){
            //イベントが無いかを判断する
            if(event.type == ENET_EVENT_TYPE_NONE){
                //イベントを設定
                m_last_event = NetEvent::NONE;
                return;
            }
            //イベントを処理する
            assert(is_there_peer(event.peer));
            //最新の受信元IDを更新する
            m_last_from = static_cast<ClientID>(event.peer - m_server->peers);
            if(event.type == ENET_EVENT_TYPE_CONNECT){
                //イベントを設定
                m_last_event = NetEvent::CONNECT;
                notify_change();
                continue;
            }
            if(event.type == ENET_EVENT_TYPE_RECEIVE){
                //受信データを更新
                m_last_recived = std::vector<uint8_t>(
                    event.packet->data,
                    event.packet->data + event.packet->dataLength
                );
                //イベントを設定
                m_last_event = NetEvent::RECEIVE;
                //パケットを削除
                enet_packet_destroy(event.packet);
                notify_change();
                continue;
            }
            if(event.type == ENET_EVENT_TYPE_DISCONNECT){
                //イベントを設定
                m_last_event = NetEvent::DISCONNECT;
                notify_change();
                continue;
            }
            assert(!"unknown event!");
        }
    };

    const NetEvent last_event() noexcept{
        return m_last_event;
    }

    const std::vector<uint8_t> last_received_data() noexcept{
        return m_last_recived;
    }

    const ClientID last_from_client_id() noexcept{
        return m_last_from;
    }

    const size_t current_connection_num() noexcept{
        return m_server->peerCount;
    }

    const bool send_to(const ClientID client_id, const size_t ch, const std::vector<uint8_t>& data) noexcept{
        //対象のENetPeerを探す．
        if(client_id >= m_server->peerCount){
            return false;
        }
        return Send_ENet_Packet(m_server->peers + client_id, ch, data, true);
    }

    const bool send_to_everyone(const size_t ch, const std::vector<uint8_t>& data) noexcept{
        return Broadcast_ENet_Packet(m_server, ch, data.data(), data.size(), true);
    }

    const bool send_to_everyone(const size_t ch, const std::string& str) noexcept{
        return Broadcast_ENet_Packet(m_server, ch, str, true);
    }

    void flush_send() noexcept{
        enet_host_flush(m_server);
    }

    private:
    ENetHost *m_server;
    std::vector<uint8_t> m_last_recived;
    NetEvent m_last_event;
    ClientID m_last_from;
};

class server_system : public Meditator, private boost::noncopyable {
    public:
    server_system() noexcept
    :m_net(nullptr)
    ,m_user(nullptr){
        create_colleagues();
    }

    ~server_system(){
        m_net = nullptr;
        m_user = nullptr;
    }

    void colleague_change(Colleague* colleague) noexcept override{
        //ここを埋める
        //入力の反映方法を探る
        if(m_net.get() == colleague){
            if(m_net->last_event() == NetEvent::CONNECT){
                m_user->add_new_user(m_net->last_from_client_id(), {.m_name = ""});
                return;
            }
            if(m_net->last_event() == NetEvent::RECEIVE){
                const auto info = m_user->get_user_info(m_net->last_from_client_id());
                assert(info.has_value());
                const auto recv = m_net->last_received_data();
                if(info->m_name == ""){
                    const user_info new_user = {
                        .m_name = std::string(recv.begin(), recv.end() - 1)
                    };
                    m_user->replace_user_info(m_net->last_from_client_id(), new_user);
                    const std::string join_message = new_user.m_name + " has connected\n";
                    m_net->send_to_everyone(0, join_message);
                    m_net->flush_send();
                    return;
                }
                const std::string user_message = info->m_name + ": "+ std::string(recv.begin(), recv.end() - 1) +"\n";
                m_net->send_to_everyone(0, user_message);
                m_net->flush_send();
                return;
            }
            if(m_net->last_event() == NetEvent::DISCONNECT){
                const auto info = m_user->get_user_info(m_net->last_from_client_id());
                m_user->remove_user_info(m_net->last_from_client_id());
                assert(info.has_value());
                const std::string disco_message = info->m_name + " has disconnected.\n";
                m_net->send_to_everyone(0, disco_message);
                m_net->flush_send();
            }
        }
    }

    void create_colleagues() noexcept override{
        m_net = std::make_unique<chat_net>(PORT, 128, 2);
        m_net->set_meditator(this);
        m_user = std::make_unique<chat_user_namager>();
        m_user->set_meditator(this);
    };

    void update() noexcept{
        m_net->update();
    }

    private:
    std::unique_ptr<chat_net> m_net;
    std::unique_ptr<chat_user_namager> m_user;
};

int  main(int argc, char ** argv) {
    if(enet_initialize() != 0) {
      printf("Could not initialize enet.");
      return 0;
    }

    auto m_server_sys = std::make_unique<server_system>();
    while(true){
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        m_server_sys->update();
    }

    enet_deinitialize();
}