#include<enet/enet.h>
#include<cassert>
#include<boost/noncopyable.hpp>
#include<string>
#include<iostream>
#include<future>
#include<memory>
#include<thread>
#include<cstdint>
#include "config.hpp"
#include "enet_send.hpp"

#include"meditator.hpp"
#include<unistd.h>
#include<msgpack.hpp>

const ENetAddress CreateENetAddress(const std::string& hostname, const enet_uint16 port){
    ENetAddress address;
    enet_address_set_host(&address, hostname.data());
    address.port = port;
    return address;
}

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
        ///文字入力をイベントハンドルに変更する
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
            this->notify_change();
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
            m_last_recived = std::vector<uint8_t>(
                event.packet->data,
                event.packet->data + event.packet->dataLength
            );
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

    template <typename T>
    const bool add_send_data(const T& data, const size_t ch) noexcept{
        if(!this->isVailed() || !this->isConnected()){
            return false;
        }
        msgpack::sbuffer send_buffer;
        msgpack::pack(send_buffer, data);
        return Send_ENet_Packet(m_server_peer, ch, send_buffer.data(), send_buffer.size(), true);
    }

    void flush() noexcept{
        if(!this->isVailed() || !this->isConnected()){
            return;
        }
        enet_host_flush(m_client);
    }

    const std::vector<uint8_t> lastReceivedData() noexcept{
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

    const bool isConnected() noexcept{
        return m_server_peer != NULL;
    }

    const void requestDisconnection() noexcept{
        enet_peer_disconnect(m_server_peer, 0);
    }

    private:
    ENetHost *m_client;
    ENetPeer *m_server_peer;
    std::vector<uint8_t> m_last_recived;
    ENetEventType m_last_event;
};

class chat_system : public Meditator , private boost::noncopyable {
    public:
    chat_system() noexcept
    :m_io(nullptr)
    ,m_communicate(nullptr)
    ,m_quit_flag(false)
    ,m_username("default_name"){
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

    const bool initilize(const std::string& usename){
        m_username = usename;
        return m_communicate->try_connection(HOST, PORT);
    }

    void colleague_change(Colleague* colleague) noexcept override{
        if(m_io.get() == colleague){
            assert(m_communicate->isVailed());
            assert(m_communicate->isConnected());
            const auto msg = m_io->last_message();
            if(msg == "q" && !m_quit_flag){
                m_communicate->requestDisconnection();
                return;
            }
            m_communicate->add_send_data(msg, 0);
            m_communicate->flush();
            return;
        }
        if(m_communicate.get() == colleague){
            assert(m_communicate->isVailed());
            const auto event = m_communicate->lastEvent();
            if(event == ENET_EVENT_TYPE_CONNECT){
                assert(m_communicate->isConnected());
                m_communicate->add_send_data(m_username, 0);
                m_communicate->flush();;
                return;
            }
            if(event == ENET_EVENT_TYPE_RECEIVE){
                assert(m_communicate->isConnected());
                const auto recv_data = m_communicate->lastReceivedData();
                msgpack::object_handle result = msgpack::unpack((const char*)recv_data.data(), recv_data.size());
                m_io->add_message("Message: " + result->as<std::string>());
                return;
            }
            if(event == ENET_EVENT_TYPE_DISCONNECT){
                assert(!m_communicate->isConnected());
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
    ///本当はColleagueに封じ込めるべきだが面倒なのでこのようにした。
    std::string m_username;
};

int  main(int argc, char ** argv) {
    if (argv[1] == NULL) {
        printf("Usage: client username\n");
        exit(1);
    }
    fprintf(stderr, "user name: %s\n", argv[1]);

    if (enet_initialize() != 0) {
        printf("Could not initialize enet.\n");
        return 0;
    }

    //チャットシステムを作成する
    auto chat_sys = std::make_unique<chat_system>();
    if(!chat_sys->initilize(argv[1])){
        chat_sys = nullptr;
        enet_deinitialize();
        return 1;
    }

    while(!chat_sys->is_quit()){
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        chat_sys->update();
    }

    chat_sys = nullptr;   
    enet_deinitialize();
    return 0;
}