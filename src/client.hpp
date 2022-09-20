#pragma once

#include<boost/noncopyable.hpp>
#include<memory>
#include"console_io.hpp"
#include"enet_client.hpp"
#include"user_info.hpp"

class chat_system : private boost::noncopyable {
    public:
    /// @brief コンストラクタ
    chat_system() noexcept;
    
    /// @brief デストラクタ
    ~chat_system();

    /// @brief 初期化関数
    /// @return 初期化結果
    const bool initilize();

    /// @brief ユーザ情報を登録する
    /// @param info ユーザ情報
    void register_user_info(const UserInfo& info);

    /// @brief サーバに接続要求を送る
    /// @param hostname ホスト名
    /// @param port ポート番号
    /// @return 接続要求送信の成否
    const bool connect_server(const std::string& hostname, const uint32_t port);

    /// @brief プログラムを終了するか
    /// @return 終了フラグ
    const bool is_quit() noexcept;

    /// @brief クライアントの更新
    void update() noexcept;

    private:
    /// @brief イベントハンドラ
    void on_io_input(const std::string& msg);
    void on_net(const ENetEvent* e);

    /// @brief 入出力関数
    std::unique_ptr<console_io> m_io;
    /// @brief ネットワークオブジェクト
    std::unique_ptr<NetClient> m_net;
    /// @brief 終了フラグ
    bool m_quit_flag;
    ///本当はColleagueに封じ込めるべきだが面倒なのでこのようにした。
    UserInfo m_info;
    /// @brief 使用するチャンネルの数
    const size_t m_ch_max = 2;
};

int  main(int argc, char ** argv);