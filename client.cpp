#include <stdio.h>
#include <string.h>
#include <enet/enet.h>
#include "config.hpp"

#include <unistd.h>

void PrintPacket(const ENetPacket* packet){
    fprintf(stderr, "[Packet] ");
    fprintf(stderr, "data: ");
    for(size_t i = 0; i < packet->dataLength; ++i){
        fprintf(stderr, "%x ", packet->data[i]);
    }
    fprintf(stderr, "\n");
}

int  main(int argc, char ** argv) {
    int connected=0;
    if (argv[1] == NULL) {
        printf("Usage: client username\n");
        exit(1);
    }
    fprintf(stderr, "user name: %s\n", argv[1]);

    if (enet_initialize() != 0) {
        printf("Could not initialize enet.\n");
        return 0;
    }

    ENetHost *client = enet_host_create(NULL, 1, 2, 5760/8, 1440/8);
    if (client == NULL) {
        printf("Could not create client.\n");
        return 0;
    }

    ENetAddress address;
    enet_address_set_host(&address, HOST);
    address.port = PORT;

    ENetPeer *peer = enet_host_connect(client, &address, 2, 0);
    if (peer == NULL) {
        printf("Could not connect to server\n");
        return 0;
    }

    ENetEvent event;
    if (enet_host_service(client, &event, 1000) > 0 && event.type == ENET_EVENT_TYPE_CONNECT) {
        printf("Connection to %s succeeded.\n", HOST);
        connected = 1;
        ENetPacket *packet = enet_packet_create(argv[1], strlen(argv[1])+1, ENET_PACKET_FLAG_RELIABLE);
        enet_peer_send(peer, 0, packet);
        enet_host_flush(client);
    } else {
        enet_peer_reset(peer);
        printf("Could not connect to %s.\n", HOST);
        return 0;
    }

    while (enet_host_service(client, &event, 1000) >= 0) {
        if(event.type == ENET_EVENT_TYPE_RECEIVE){
            printf("%s\n", (char*) event.packet->data);
            enet_packet_destroy(event.packet);
            continue;
        }
        if(event.type == ENET_EVENT_TYPE_DISCONNECT){
            connected = 0;
            printf("You have been disconnected.\n");
            break;
        }
        if(event.type == ENET_EVENT_TYPE_NONE){
            if (connected) {
                printf("Input: ");
                const char *buffer = "hi all";
                //char  buffer[BUFFERSIZE] = { 0 };
                //scanf("%[^\n]%*c", buffer);
                if (strlen(buffer) == 0) { continue; }
                if (strlen(buffer) == 1 && buffer[0] == 'q') {
                    connected = 0;
                    enet_peer_disconnect(peer, 0);
                    continue;
                }

                ENetPacket *packet = enet_packet_create(buffer, strlen(buffer)+1, ENET_PACKET_FLAG_RELIABLE);
                enet_peer_send(peer, 0, packet);
            }
        }
    }

    enet_peer_reset(peer);
    enet_host_destroy(client);
    enet_deinitialize();
}