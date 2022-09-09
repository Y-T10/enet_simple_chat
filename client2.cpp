#include<cassert>
#include<boost/noncopyable.hpp>
#include<string>
#include<iostream>
#include<future>
#include<memory>
#include<cstdint>
#include "config.hpp"
#include "enet_client.hpp"
#include<functional>

#include"meditator.hpp"
#include<unistd.h>
#include<msgpack.hpp>
#include<boost/container_hash/hash.hpp>

const ENetAddress CreateENetAddress(const std::string& hostname, const enet_uint16 port){
    ENetAddress address;
    enet_address_set_host(&address, hostname.data());
    address.port = port;
    return address;
}

class chat_io {
    public:
    chat_io() noexcept
    :m_message_receiver(){};
    ~chat_io() = default;

    void add_message(const std::string& message) noexcept{
        std::cout << message << std::endl;
    }

    void handle_input(const std::function<void(const std::string&)>& read_handle) noexcept{
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
            read_handle(m_message_receiver.get());
            m_message_receiver = std::async(std::launch::async, read_cin);
        }
    }
    private:
    std::future<std::string> m_message_receiver;
};

class chat_system : private boost::noncopyable {
    public:
    chat_system() noexcept
    :m_io(nullptr)
    ,m_net(nullptr)
    ,m_quit_flag(false)
    ,m_username("default_name"){
    }

    ~chat_system(){
        m_io = nullptr;
        m_net = nullptr;
    }

    const bool initilize(const std::string& usename){
        m_username = usename;
        m_io = std::make_unique<chat_io>();
        m_net = std::make_unique<NetClient>();
        m_net->set_host({1, 2, 0, 0, NULL});
        return m_net->request_connection(CreateENetAddress(HOST, PORT), 2, 1000);
    }

    const bool is_quit() noexcept{
        return m_quit_flag;
    }

    void update() noexcept{
        using namespace std::placeholders;
        //文字列入力を処理する
        m_io->handle_input(std::bind(&chat_system::on_io_input, this, _1));
        //ネットワークのイベントを処理する
        m_net->handle_host_event(std::bind(&chat_system::on_net, this, _1));
    }

    private:
    void on_io_input(const std::string& msg){
        if(msg == "q" && !m_quit_flag){
            m_net->close_all();
            return;
        }
        m_net->handle_peer(
            [](const ENetPeer* p)->const bool{return (uintptr_t)(p->data) != (uintptr_t)(0xff);},
            [this,&msg](ENetPeer* p){
                ENetPacket* packet = enet_packet_create(msg.data(), msg.size(), ENET_PACKET_FLAG_RELIABLE);
                enet_peer_send(p, 0,packet);
            });
        m_net->flush();
        return;
    }

    void on_net(const ENetEvent* e){
        if(e->type == ENET_EVENT_TYPE_NONE){
            std::cerr << "waiting for a new event...\r";
            return;
        }
        if(e->type == ENET_EVENT_TYPE_CONNECT){
            e->peer->data = (void*)(uintptr_t)(0xff);
            return;
        }
        if(e->type == ENET_EVENT_TYPE_RECEIVE){
            using namespace msgpack;
            object_handle result = unpack((const char*)e->packet->data, e->packet->dataLength);
            m_io->add_message("Message: " + result->as<std::string>());
            return;
        }
        if(e->type == ENET_EVENT_TYPE_DISCONNECT){
            if(e->peer->data != NULL){
                m_io->add_message("disconnected.");
            }else{
                m_io->add_message("connection failed.");
            }
            e->peer->data = NULL;
            m_quit_flag = true;
            return;
        }
    };

    std::unique_ptr<chat_io> m_io;
    std::unique_ptr<NetClient> m_net;
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

    while(chat_sys->is_quit() == false){
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        chat_sys->update();
    }

    chat_sys = nullptr;   
    enet_deinitialize();
    return 0;
}