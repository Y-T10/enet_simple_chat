#include <stdio.h>
#include <string.h>
#include <enet/enet.h>
#include<cassert>
#include<boost/noncopyable.hpp>
#include<string>
#include<iostream>
#include<functional>
#include<cstddef>
#include<future>
#include<memory>
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

const ENetAddress CreateENetAddress(const std::string& hostname, const enet_uint16 port){
    ENetAddress address;
    enet_address_set_host(&address, hostname.data());
    address.port = port;
    return address;
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
    chat_io() noexcept
    :m_latest_message("")
    ,m_message_receiver(){};
    ~chat_io() = default;

    const std::string last_message() noexcept{
        return m_latest_message;
    }
    void add_message(const std::string& message) noexcept{
        std::cout << message << std::endl;
    }
    void update() noexcept{
        const auto read_cin = []() -> std::string {
            std::string message = "";
            std::cin >> message;
            return message;
        };
        if(!m_message_receiver.valid()){
            m_message_receiver = std::async(std::launch::async, read_cin);
        }
        if(m_message_receiver.wait_for(std::chrono::seconds(0)) == std::future_status::ready){
            m_latest_message = m_message_receiver.get();
            m_message_receiver = std::async(std::launch::async, read_cin);
            notify_change();
        }
    }
    private:
    std::string m_latest_message;
    std::future<std::string> m_message_receiver;
};

class chat_communication : public Colleague{
    public:
    chat_communication() noexcept
    :m_client(enet_host_create(NULL, 1, 2, 5760/8, 1440/8))
    ,m_server_peer(NULL)
    ,m_last_recived()
    ,m_last_event(ENET_EVENT_TYPE_NONE){};
    ~chat_communication(){
        if(m_server_peer != NULL){
            enet_peer_reset(m_server_peer);
        }
        if(m_client != NULL){
            enet_host_destroy(m_client);
        }
    }

    const bool try_connection(const std::string& hostname, const enet_uint16 port) noexcept{
        m_last_event = ENET_EVENT_TYPE_NONE;
        if(!this->isVailed()){
            return false;
        }
        if(m_server_peer != NULL){
            return false;
        }
        const ENetAddress address = CreateENetAddress(hostname, port);
        m_server_peer = enet_host_connect(m_client, &address, 2, 0);
        if (m_server_peer == NULL) {
            return false;
        }

        ENetEvent event;
        if(enet_host_service(m_client, &event, 1000) > 0 && event.type == ENET_EVENT_TYPE_CONNECT) {
            assert(m_server_peer == event.peer);
            m_last_event = ENET_EVENT_TYPE_CONNECT;
            return true;
        }
        enet_peer_reset(m_server_peer);
        m_server_peer = NULL;
        return false;
    };

    void update() noexcept{
        if(!this->isVailed()){
            return;
        }
        ENetEvent event;
        if(enet_host_service(m_client, &event, 0) < 0){
            return;
        }
        assert(event.type != ENET_EVENT_TYPE_CONNECT);
        m_last_event = event.type;
        if(event.type == ENET_EVENT_TYPE_RECEIVE){
            const enet_uint8* recv_begin = event.packet->data;
            const size_t recv_length = event.packet->dataLength;
            const std::vector<std::byte> received(recv_begin, recv_begin + recv_length);
            m_last_recived = std::move(received);
            enet_packet_destroy(event.packet);
            notify_change();
            return;
        }
        if(event.type == ENET_EVENT_TYPE_DISCONNECT){
            assert(event.peer == m_server_peer);
            assert(m_server_peer != NULL);
            enet_peer_reset(m_server_peer);
            m_server_peer = NULL;
            notify_change();
            return;
        }
    }

    const bool add_sent_data(const std::vector<std::byte>& data, const size_t ch) noexcept{
        if(!this->isVailed() || m_server_peer == NULL){
            return false;
        }
        return Send_ENet_Packet(m_server_peer, ch, data, true);
    }

    void flush() noexcept{
        if(!this->isVailed() || m_server_peer == NULL){
            return;
        }
        enet_host_flush(m_client);
    }

    const std::vector<std::byte> lastReceivedData() noexcept{
        return m_last_recived;
    }

    const ENetEventType lastEvent() noexcept{
        return m_last_event;
    }

    const bool isVailed() noexcept{
        if(m_client == NULL){
            return false;
        }
        return true;
    }

    private:
    ENetHost *m_client;
    ENetPeer *m_server_peer;
    std::vector<std::byte> m_last_recived;
    ENetEventType m_last_event;
};

class chat_system : public Meditator , private boost::noncopyable {
    public:
    chat_system() noexcept
    :m_io(nullptr)
    ,m_communicate(nullptr)
    ,m_quit_flag(false){
        create_colleagues();
    }

    ~chat_system(){
        m_io = nullptr;
        m_communicate = nullptr;
    }

    void create_colleagues() noexcept override{
        m_io = std::make_unique<chat_io>();
        m_io->set_meditator(this);
        m_communicate = std::make_unique<chat_communication>();
        m_communicate->set_meditator(this);
    };

    const bool initilize(){
        return m_communicate->try_connection(HOST, PORT);
    }

    void colleague_change(Colleague* colleague) noexcept override{
        if(m_io.get() == colleague){
            const auto msg = m_io->last_message();
            const auto msg_vector = std::vector<std::byte>(msg.begin(), msg.end());
            m_communicate->add_sent_data(msg_vector, 0);
            m_communicate->flush();
            return;
        }
        if(m_communicate.get() == colleague){
            assert(m_communicate->isVailed());
            const auto event = m_communicate->lastEvent();
            if(event == ENET_EVENT_TYPE_RECEIVE){
                const auto recv_data = m_communicate->lastReceivedData();
                m_io->add_message("Message: " + std::string(recv_data.begin(), recv_data.end()));
                return;
            }
            if(event == ENET_EVENT_TYPE_DISCONNECT){
                m_io->add_message("disconnected.");
                m_quit_flag = true;
                return;
            }
        }
    }

    const bool is_quit() noexcept{
        return m_quit_flag;
    }

    void update() noexcept{
        m_io->update();
        m_communicate->update();
    }

    private:
    std::unique_ptr<chat_io> m_io;
    std::unique_ptr<chat_communication> m_communicate;
    bool m_quit_flag;
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