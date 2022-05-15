#include"enet_send.hpp"

const bool Send_ENet_Packet(ENetPeer* dst, const size_t channel, const void* data, const size_t data_size, const bool is_reliable){
    const enet_uint32 flag = is_reliable ? ENET_PACKET_FLAG_RELIABLE : ENET_PACKET_FLAG_UNRELIABLE_FRAGMENT;
    ENetPacket *packet = enet_packet_create(data, data_size, flag);
    if(packet == NULL){
        return false;
    }
    if(enet_peer_send(dst, channel, packet) < 0){
        enet_packet_destroy(packet);
        return false;
    }
    return true;
}