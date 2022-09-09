#pragma once

#include<enet/enet.h>
#include<cstdint>
#include<vector>
#include<tuple>
#include<boost/noncopyable.hpp>
#include<functional>

using RecvData = std::vector<uint8_t>;
using PacketCh = decltype(ENetEvent::channelID);


/// @brief ホストの設定
struct HostSetting {
    /// @brief 最大接続数
    size_t peer_max;
    /// @brief 最大パケットチャンネル数
    size_t packet_ch_max;
    /// @brief 毎秒受信バイトサイズ
    enet_uint32 down_size;
    /// @brief 毎秒送信バイトサイズ
    enet_uint32 up_size;
    /// @brief ホストのアドレス
    ENetAddress* address;
};

class basic_enet {
    public:
    /// @brief 既定のコンストラクタ
    basic_enet();

    /// @brief デストラクタ
    ~basic_enet();

    ///状態確認用演算子
    ///@return エラーの有無を返す
    operator bool();

    /// @brief ホストを設定する
    /// @param settings ホストの設定
    /// @return 設定の成否
    const bool set_host(const HostSetting& settings);

    /// @brief ホストを削除する
    void clear_host();

    /// @brief 接続を要求する
    /// @param dst 接続先のアドレス
    /// @param max_ch 使用するチャンネルの数
    /// @param data 接続先に与える値
    /// @return 接続要求の成否
    const bool request_connection(const ENetAddress& dst, const size_t max_ch, const enet_uint32 data);

    /// @brief 目的の接続相手の有無を調べる
    /// @param finder 接続相手かを答える関数
    /// @return 有無の結果
    const bool is_there_peer(const std::function<const bool(const ENetPeer*)>& finder);

    /// @brief 目的の接続相手に対する処理を行う
    /// @param finder 目的の接続相手かを答える関数
    /// @param peer_handler 接続相手を扱う関数．目的の相手が見つかった時に呼び出される．
    void handle_peer(
        const std::function<const bool(const ENetPeer*)>& finder,
        const std::function<void(ENetPeer*)>& peer_handler
    );

    ///ホストのイベントを処理する
    ///@param event_handler イベントを処理する関数
    ///@return イベント処理の結果
    const bool handle_host_event(const std::function<void(const ENetEvent*)>& event_handler);

    /// @brief 全接続先にパケットを送る
    /// @param ch 送信チャンネル
    /// @param data 送信パケット
    /// @return 送信結果
    void broadcast(const size_t ch, ENetPacket* packet);

    ///データを実際に送る
    void flush();

    ///全接続を切る
    void close_all();

    private:
    /// @brief 接続先とやり取りするホスト
    ENetHost *m_host;
};