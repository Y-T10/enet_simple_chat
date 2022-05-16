#pragma once

#include<enet/enet.h>
#include<string>
#include<thread>
#include<boost/lockfree/spsc_queue.hpp>
#include<boost/noncopyable.hpp>
#include<variant>

class ENetClient : private boost::noncopyable {
    public:
    /**
     * コンストラクタ
     * @param ch_count チャンネル数
     * @param down_band_width サーバからの帯域幅。byte/s単位。
     * @param up_band_width サーバへの帯域幅。byte/s単位。
     */
    explicit ENetClient(
        const size_t ch_count = 1,
        const enet_uint32 down_band_width = 0,
        const enet_uint32 up_band_width = 0
    ) noexcept;

    ///デストラクタ
    ~ENetClient();

    ///状態確認関数
    ///@return 正常か否か
    const bool IsVailed() noexcept;

    //接続関数
    const bool request_connection(
        const std::string& host_name,
        const enet_uint16 port
    ) noexcept;

    private:
    ///ムーブを禁止する
    ENetClient(ENetClient&& rval) = delete;
    ENetClient& operator=(ENetClient &&rval) = delete;

    private:
    struct req_connect {
        std::string host_name;
        enet_uint16 port;
    };
    using request_event = std::variant<req_connect>;

    ///クライアントの実体
    ENetHost *m_client;
    ///通信を行うスレッド
    std::thread m_communicate_thread;
    ///スレッド間通信用キュー
    boost::lockfree::spsc_queue<request_event> m_reqest_queue;
};