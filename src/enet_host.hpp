#pragma once

#include"basic_enet.hpp"

class NetHost : public basic_enet {
    public:
    /// @brief コンストラクタ
    NetHost() noexcept;
    /// @brief デストラクタ
    ~NetHost();

    /// @brief 全接続先にパケットを送る
    /// @param ch 送信チャンネル
    /// @param data 送信パケット
    /// @return 送信結果
    void broadcast(const size_t ch, ENetPacket* packet);
};