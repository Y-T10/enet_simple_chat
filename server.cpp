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
    :m_user_list(0){};
    ~chat_user_namager() noexcept{
    }

    const bool add_new_user(const ClientID user_id, const user_info& info) noexcept{
        if(m_user_list.size() < user_id || m_user_list[user_id].has_value()){
            return false;
        }
        if(m_user_list.size() == user_id){
            m_user_list.emplace_back(std::nullopt);
        }
        assert(user_id < m_user_list.size());
        assert(m_user_list[user_id] == std::nullopt);
        m_user_list[user_id] = info;
    }

    const std::optional<user_info> get_user_info(const ClientID user_id) noexcept{
        if(m_user_list.size() > user_id){
            return std::nullopt;
        }
        assert(m_user_list[user_id].has_value());
        return m_user_list[user_id];
    };

    void replace_user_info(const ClientID user_id, const user_info& info) noexcept{
        if(m_user_list.size() <= user_id || !m_user_list[user_id].has_value()){
            return;
        }
        m_user_list[user_id] = info;
    };

    void remove_user_info(const ClientID user_id) noexcept{
        if(m_user_list.size() > user_id){
            return;
        }
        m_user_list[user_id] = std::nullopt;
    };

    const size_t user_num() noexcept{
        const auto is_null = [](const std::optional<user_info>& info){return info != std::nullopt;};
        return ranges::count_if(m_user_list, is_null);
    }

    private:
    std::vector<std::optional<user_info>> m_user_list;
};

class chat_net : public Colleague {
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
            assert(any_of(m_server->peers, m_server->peers + m_server->peerCount, event.peer));
            assert(event.peer > m_server->peers);
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

    private:
    ENetHost *m_server;
    std::vector<uint8_t> m_last_recived;
    NetEvent m_last_event;
    ClientID m_last_from;
};

class server_system : public Meditator, private boost::noncopyable {
    public:
    server_system() noexcept
    :m_net(nullptr){
        create_colleagues();
    }

    ~server_system(){
        m_net = nullptr;
    }

    void colleague_change(Colleague* colleague) noexcept override{
        //ここを埋める
        //入力の反映方法を探る
        if(m_net.get() == colleague){
        }
    }

    void create_colleagues() noexcept override{
        m_net = std::make_unique<chat_net>(PORT, 128, 2);
        m_net->set_meditator(this);
    };

    private:
    std::unique_ptr<chat_net> m_net;
};

int  main(int argc, char ** argv) {
    if(enet_initialize() != 0) {
      printf("Could not initialize enet.");
      return 0;
    }
    const ENetAddress address = {.host = ENET_HOST_ANY, .port = PORT};
    ENetHost *server = enet_host_create(&address, 100, 2, 0, 0);
    if(server == NULL) {
      printf("Could not start server.\n");
      return 0;
    }

    ENetEvent event;
    while (enet_host_service(server, &event, 1000) >= 0) {
        if(event.type == ENET_EVENT_TYPE_CONNECT){
            const enet_uint32 ip = event.peer->address.host;
            const enet_uint16 port = event.peer->address.port;
            fprintf(stderr, "[connection] ip: %x, port: %u\n", ip, port);
            continue;
        }
        if(event.type == ENET_EVENT_TYPE_DISCONNECT){
            fprintf(stderr, "[disconnect] \"%s\" disconnect.\n", (char*)event.peer->data);
            char buffer[BUFFERSIZE] = { 0 };
            sprintf(buffer, "%s has disconnected.", (char*)event.peer->data);
            Broadcast_ENet_Packet(server, 0, buffer, strlen(buffer)+1, false);
            enet_host_flush(server);
            free(event.peer->data);
            event.peer->data = NULL;
            continue;
        }
        if(event.type == ENET_EVENT_TYPE_RECEIVE){
            PrintPacket(event.packet);
            if (event.peer->data == NULL) {
                fprintf(stderr, "[join] \"%s\" joined.\n", event.packet->data);
                char buffer[BUFFERSIZE] = { 0 };
                event.peer->data = malloc(strlen((char*) event.packet->data)+1);
                strcpy((char*) event.peer->data, (char*) event.packet->data);
                sprintf(buffer, "%s has connected\n", (char*) event.packet->data);
                Broadcast_ENet_Packet(server, 0, buffer, strlen(buffer)+1, false);
                enet_host_flush(server);
            } else {
                char buffer[BUFFERSIZE] = { 0 };
                sprintf(buffer, "%s: %s", (char*) event.peer->data, (char*)event.packet->data);
                Broadcast_ENet_Packet(server, 0, buffer, strlen(buffer)+1, false);
                enet_host_flush(server);
                fprintf(stderr, "[message] %s\n", buffer);
            }
            enet_packet_destroy(event.packet);
            continue;
        }
    }
    enet_host_destroy(server);
    enet_deinitialize();
}