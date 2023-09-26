#include"console_io.hpp"
#include<iostream>

console_io::console_io() noexcept
:m_message_receiver(){};
console_io::~console_io(){
    m_message_receiver = std::future<std::string>();
}

void console_io::add_message(const std::string& message) noexcept{
    std::cout << message << std::endl;
}

void console_io::handle_input(const std::function<void(const std::string&)>& read_handle) noexcept{
    ///文字入力をイベントハンドルに変更する
    const auto read_cin = []() -> std::string {
        std::string message = "";
        std::cin >> message;
        return message;
    };
    if(!m_message_receiver.valid()){
        m_message_receiver = std::async(std::launch::async, read_cin);
    }
    if(m_message_receiver.wait_for(std::chrono::seconds(0)) == std::future_status::ready){
        read_handle(m_message_receiver.get());
        m_message_receiver = std::async(std::launch::async, read_cin);
    }
}