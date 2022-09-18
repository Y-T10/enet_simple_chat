#pragma once

#include<enet/enet.h>
#include<boost/noncopyable.hpp>
#include<msgpack.hpp>

class PacketStream : private boost::noncopyable {
    public:
    /// @brief コンストラクタ
    PacketStream() noexcept;

    ///デストラクタ
    ~PacketStream();

    /// @brief 値を書き込む
    /// @tparam T 値の型
    /// @param value 値
    template <typename T>
    void write(const T& value) noexcept{
        msgpack::pack(m_buffer, value);
    };

    /// @brief リアルタイムで送信されるパケットに書き出す
    /// @return 書き出されたパケット
    ENetPacket* realtime_packet() noexcept;

    /// @brief パケットに書き出す
    /// @return 書き出されたパケット
    ENetPacket* packet() noexcept;

    private:
    /// @brief 値が書き込まれるバッファ
    msgpack::sbuffer m_buffer;
};