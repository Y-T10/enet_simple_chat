#include <stdio.h>
#include <string.h>
#include <enet/enet.h>
#include<cassert>
#include<boost/noncopyable.hpp>
#include<string>
#include<iostream>
#include "config.hpp"
#include "enet_send.hpp"

#include <unistd.h>

void PrintPacket(const ENetPacket* packet){
    fprintf(stderr, "[Packet] ");
    fprintf(stderr, "data: ");
    for(size_t i = 0; i < packet->dataLength; ++i){
        fprintf(stderr, "%x ", packet->data[i]);
    }
    fprintf(stderr, "\n");
}

class Colleague : private boost::noncopyable {
    public:
    Colleague() noexcept
    :m_meditator(nullptr){};
    virtual ~Colleague() = default;

    virtual void set_meditator(Meditator* meditator) noexcept{
        assert(meditator != nullptr);
        m_meditator = meditator;
    }
    virtual void notify_change() noexcept{
        assert(m_meditator != nullptr);
        m_meditator->colleague_change(this);
    }
    private:
    Meditator* m_meditator;
};

class Meditator {
    public:
    Meditator() noexcept = default;
    virtual ~Meditator() = default;

    virtual void create_colleagues() noexcept = 0;
    virtual void colleague_change(Colleague* colleague) noexcept = 0;
};

class chat_io : public Colleague {
    public:
    chat_io() noexcept = default;
    ~chat_io() = default;

    const std::string fetch_latest_message() noexcept{
        return m_latest_message;
    }
    void add_message(const std::string& message) noexcept{
        std::cout << message << std::endl;
    }
    void write_message() noexcept{
        std::cin >> m_latest_message;
        notify_change();
    }
    private:
    std::string m_latest_message;
};

class chat_system : public Meditator {
    public:
    void create_colleagues() noexcept override{
        m_io = new chat_io;
        m_io->set_meditator(this);
    };

    void colleague_change(Colleague* colleague) noexcept override{
        if(m_io == colleague){
            const auto msg = m_io->fetch_latest_message();
        }
    }

    private:
    chat_io *m_io;
};

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
        Send_ENet_Packet(peer, 0, argv[1], strlen(argv[1])+1, true);
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
                //printf("Input: ");
                const char *buffer = "hi all";
                //char  buffer[BUFFERSIZE] = { 0 };
                //scanf("%[^\n]%*c", buffer);
                if (strlen(buffer) == 0) { continue; }
                if (strlen(buffer) == 1 && buffer[0] == 'q') {
                    connected = 0;
                    enet_peer_disconnect(peer, 0);
                    continue;
                }
                Send_ENet_Packet(peer, 0, buffer, strlen(buffer)+1, false);
            }
        }
    }

    enet_peer_reset(peer);
    enet_host_destroy(client);
    enet_deinitialize();
}