#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <enet/enet.h>
#include "config.hpp"
#include "enet_send.hpp"
#include "meditator.hpp"
#include<algorithm>

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

class chat_net : public Colleague {
    public:

    explicit chat_net(const enet_uint16 port, const size_t peer_max, const size_t ch) noexcept
    :m_server(NULL)
    ,m_client_id_usage(peer_max)
    ,m_last_recived(0)
    ,m_last_event(NetEvent::NONE)
    ,m_last_from(0){
        const ENetAddress address = {.host = ENET_HOST_ANY, .port = port};
        m_server = enet_host_create(&address, peer_max, ch, 0, 0);
        m_client_id_usage.resize(peer_max);
    }

    const bool is_valied() noexcept{
        return m_server != NULL;
    }

    void update(){
        ///イベントを受け取る
        ENetEvent event;
        while(enet_host_service(m_server, &event, 0) >= 0){
            assert(m_server->peerCount == static_cast<size_t>(std::ranges::count(m_client_id_usage, true)));

            if(event.type == ENET_EVENT_TYPE_NONE){
                //イベントを設定
                m_last_event = NetEvent::NONE;
                continue;
            }
            if(event.type == ENET_EVENT_TYPE_CONNECT){
                //新しいクライアントを登録できるかを試みる
                auto foundID = std::ranges::find(m_client_id_usage, false);
                if(foundID == m_client_id_usage.cend()){
                    //満員なので切断する．
                    enet_peer_disconnect(event.peer, 0);
                    //イベントを設定
                    m_last_event = NetEvent::REFUSE;
                    continue;
                }
                //IDを使用中にする
                *foundID = true;
                //新しいID
                const ClientID new_id = static_cast<ClientID>(foundID - m_client_id_usage.begin());
                //割り当てたIDを記録させる
                event.peer->data = reinterpret_cast<void*>(new_id);
                //最新の受信元IDを更新する
                m_last_from = new_id;
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
                //受信元を更新
                m_last_from = reinterpret_cast<ClientID>(event.peer->data);
                assert(m_last_from < m_client_id_usage.size());
                assert(m_client_id_usage.at(m_last_from));
                //イベントを設定
                m_last_event = NetEvent::RECEIVE;
                //パケットを削除
                enet_packet_destroy(event.packet);
                notify_change();
                continue;
            }
            if(event.type == ENET_EVENT_TYPE_DISCONNECT){
                //受信元を更新
                m_last_from = reinterpret_cast<ClientID>(event.peer->data);
                assert(m_last_from < m_client_id_usage.size());
                assert(m_client_id_usage.at(m_last_from));
                //割り当てたIDを削除
                event.peer->data = NULL;
                //IDを開放
                m_client_id_usage[m_last_from] = false;
                //イベントを設定
                m_last_event = NetEvent::DISCONNECT;
                notify_change();
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
        //対象のENetPeerを探す．キャッシュしたほうが早いかも．
        const auto target_peer = std::find_if(
            m_server->peers, m_server->peers + m_server->peerCount,
            [&client_id](const ENetPeer& peer) -> bool { return (ClientID)(peer.data) == client_id;});
        if(target_peer == m_server->peers + m_server->peerCount){
            return false;
        }
        return Send_ENet_Packet(target_peer, ch, data, true);
    }

    const bool send_to_everyone(const size_t ch, const std::vector<uint8_t>& data) noexcept{
        return Broadcast_ENet_Packet(m_server, ch, data.data(), data.size(), true);
    }

    private:
    ENetHost *m_server;
    std::vector<bool> m_client_id_usage;
    std::vector<uint8_t> m_last_recived;
    NetEvent m_last_event;
    ClientID m_last_from;
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