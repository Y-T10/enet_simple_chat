#include<cassert>
#include<boost/noncopyable.hpp>
#include<iostream>
#include<memory>
#include<cstdint>
#include"console_io.hpp"
#include"config.hpp"
#include"enet_client.hpp"
#include"enet_utils.hpp"
#include"enet_packet_stream.hpp"
#include<functional>
#include<thread>
#include<msgpack.hpp>

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
        m_io = std::make_unique<console_io>();
        m_net = std::make_unique<NetClient>();
        m_net->set_host({1, 2, 0, 0, NULL});
        return m_net->request_connection(CreateENetAddress(HOST, PORT), 2, 0);
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
            [](const ENetPeer* p)->const bool{return (uintptr_t)(p->data) == (uintptr_t)(0xff);},
            [this,&msg](ENetPeer* p){
                PacketStream pstream;
                pstream.write(msg);
                enet_peer_send(p, 0,pstream.packet());
            });
        m_net->flush();
        return;
    }

    void on_net(const ENetEvent* e){
        if(e->type == ENET_EVENT_TYPE_NONE){
            return;
        }
        if(e->type == ENET_EVENT_TYPE_CONNECT){
            e->peer->data = (void*)(uintptr_t)(0xff);
            PacketStream pstream;
            pstream.write(m_username);
            enet_peer_send(e->peer, 0,pstream.packet());
            m_net->flush();
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

    std::unique_ptr<console_io> m_io;
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