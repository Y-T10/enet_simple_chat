#pragma once

#include<boost/unordered_map.hpp>
#include"enet_packet_stream.hpp"
#include"user_info.hpp"
#include"sig_event.hpp"
#include"enet_host.hpp"
#include<memory>

class server_system : private boost::noncopyable {
    public:
    /// @brief コンストラクタ
    server_system() noexcept;

    /// @brief デストラクタ
    ~server_system();

    /// @brief 初期化する
    void init() noexcept;

    /// @brief サーバを立ちあげる
    /// @param port ポート番号
    /// @return 立ち上げの成否
    const bool setup_server(const enet_uint16 port) noexcept;

    /// @brief サーバを更新する
    void update() noexcept;

    /// @brief サーバの終了を調べる
    /// @return 終了の是非
    const bool isQuit() noexcept;

    private:
    /// @brief イベントハンドラ
    void on_connect(const ENetEvent* e);
    void on_recv(const ENetEvent* e);
    void on_disconnect(const ENetEvent* e);
    void on_signal(const int sig);
    void on_request(ENetPeer* peer, PacketUnpacker& unpacker);
    void on_message(ENetPeer* peer, PacketUnpacker& unpacker);

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