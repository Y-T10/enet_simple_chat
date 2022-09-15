#pragma once

#include"basic_enet.hpp"
#include<optional>
#include<future>

class NetClient : public basic_enet {
    public:
    /// @brief コンストラクタ
    NetClient() noexcept;
    /// @brief デストラクタ
    ~NetClient();

    /// @brief 接続を要求する
    /// @param dst 接続先のアドレス
    /// @param max_ch 使用するチャンネルの数
    /// @param data 接続先に渡す値
    /// @return 接続要求送信の成否
    const bool request_connection(const ENetAddress& dst, const size_t max_ch, const enet_uint32 data);
};