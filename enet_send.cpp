#include"enet_send.hpp"

#include<ranges>

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

const bool Send_ENet_Packet(ENetPeer* dst, const size_t channel, const std::vector<uint8_t>& data, const bool is_reliable){
    return Send_ENet_Packet(dst, channel, data.data(), data.size(), is_reliable);
}

const bool Send_ENet_Packet(ENetPeer* dst, const size_t channel, const std::string& data, const bool is_reliable){
    return Send_ENet_Packet(dst, channel, data.data(), data.size(), is_reliable);
}

const bool Broadcast_ENet_Packet(ENetHost* server, const size_t channel, const void* data, const size_t data_size, const bool is_reliable){
    if(server == NULL){
        return false;
    }

    const enet_uint32 flag = is_reliable ? ENET_PACKET_FLAG_RELIABLE : ENET_PACKET_FLAG_UNRELIABLE_FRAGMENT;
    ENetPacket *packet = enet_packet_create(data, data_size, flag);
    if(packet == NULL){
        return false;
    }
    enet_host_broadcast(server, channel, packet);
    return true;
}