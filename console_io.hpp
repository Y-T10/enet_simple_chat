#pragma once

#include<boost/noncopyable.hpp>
#include<string>
#include<future>
#include<functional>

class console_io : private boost::noncopyable {
    public:
    ///コンストラクタ
    console_io() noexcept;
    ///デストラクタ
    ~console_io();

    /// @brief 文字列を出力する
    /// @param message 出力メッセージ
    void add_message(const std::string& message) noexcept;

    /// @brief 入力を処理する
    /// @param read_handle 入力を受け取る関数 
    void handle_input(const std::function<void(const std::string&)>& read_handle) noexcept;

    private:
    ///入力を受け取る
    std::future<std::string> m_message_receiver;
};