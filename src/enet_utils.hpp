#pragma once

#include <enet/enet.h>
#include <cstdio>

void PrintPacket(const ENetPacket* packet){
    fprintf(stderr, "[Packet] ");
    fprintf(stderr, "data: ");
    for(size_t i = 0; i < packet->dataLength; ++i){
        fprintf(stderr, "%x ", packet->data[i]);
    }
    fprintf(stderr, "\n");
}

const ENetAddress CreateENetAddress(const std::string& hostname, const enet_uint16 port){
    ENetAddress address;
    enet_address_set_host(&address, hostname.data());
    address.port = port;
    return address;
}