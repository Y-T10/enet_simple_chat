#pragma once

#include<boost/unordered_map.hpp>
#include"enet_packet_stream.hpp"
#include"user_info.hpp"
#include"sig_event.hpp"
#include"enet_host.hpp"
#include<memory>

class server_system : private boost::noncopyable {
    public:
    server_system() noexcept;

    ~server_system();

    void init() noexcept;

    void update() noexcept;

    const bool isQuit() noexcept;

    private:
    /// @brief イベントハンドラ
    void on_connect(const ENetEvent* e);
    void on_recv(const ENetEvent* e);
    void on_disconnect(const ENetEvent* e);
    void on_signal(const int sig);

    private:
    /// @brief PeerのID
    using ENetPeerID = ENetPeer*;
    /// @brief ネットオブジェクト
    std::unique_ptr<NetHost> m_net;
    /// @brief シグナルオブジェクト
    std::unique_ptr<SigEvent> m_sig;
    /// @brief ユーザリスト
    boost::unordered_map<ENetPeerID, UserInfo> m_user;
    /// @brief プログラムを終了するかのフラグ
    bool m_isQuit;
};

int  main(int argc, char ** argv);