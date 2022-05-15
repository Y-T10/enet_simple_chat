#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <enet/enet.h>
#include "config.hpp"
#include "enet_send.hpp"

void PrintPacket(const ENetPacket* packet){
    fprintf(stderr, "[Packet] ");
    fprintf(stderr, "data: ");
    for(size_t i = 0; i < packet->dataLength; ++i){
        fprintf(stderr, "%x ", packet->data[i]);
    }
    fprintf(stderr, "\n");
}

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
            for (size_t i = 0; i < server->peerCount; i++) {
                if(&server->peers[i] != event.peer) {
                    Send_ENet_Packet(&server->peers[i], 0, buffer, strlen(buffer)+1, false);
                }
            }
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
                ENetPacket *packet = enet_packet_create(buffer, strlen(buffer)+1, 0);
                enet_host_broadcast(server, 0, packet);
                enet_host_flush(server);
            } else {
                char buffer[BUFFERSIZE] = { 0 };
                sprintf(buffer, "%s: %s", (char*) event.peer->data, (char*)event.packet->data);
                for (size_t i = 0; i < server->peerCount; i++) {
                    if(&server->peers[i] != event.peer) {
                        Send_ENet_Packet(&server->peers[i], 0, buffer, strlen(buffer)+1, false);
                    }
                }
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