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
    /// @param timeout 接続要求がタイムアウト
    /// @return 接続要求送信の成否
    const bool request_connection(const ENetAddress& dst, const size_t max_ch, const enet_uint32 timeout);
    
    /// @brief 接続要求の結果を調べる
    /// @return 接続要求の成否
    const std::optional<bool> check_connection_result();
    private:
    /// @brief 接続要求の結果を受け取る
    std::future<bool> m_request_result;
};