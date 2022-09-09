#include"enet_client.hpp"
#include<cassert>

NetClient::NetClient() noexcept
:basic_enet(),
m_request_result(){
}

NetClient::~NetClient(){}

const bool NetClient::request_connection(const ENetAddress& dst, const size_t max_ch, const enet_uint32 timeout){
    assert(*this);
    //既に接続要求しているかを判断する
    if(m_request_result.valid()){
        return false;
    }
    //接続要求を送る
    if(enet_host_connect(m_host, &dst, max_ch, 0) == NULL){
        return false;
    }
    //接続要求の結果を後で受け取れるようにする
    m_request_result = std::async(std::launch::async, [this,timeout]() -> bool {
         ENetEvent event;
        if(enet_host_service(m_host, &event, timeout) > 0 && event.type == ENET_EVENT_TYPE_CONNECT) {
            return true;
        }
        enet_peer_reset(event.peer);
        return false;
    });
    return true;
}

const std::optional<bool> NetClient::check_connection_result(){
    //接続要求は送られていない
    if(!m_request_result.valid()){
        return std::nullopt;
    }
    //接続要求の結果が出たかを判断する
    if(m_request_result.wait_for(std::chrono::seconds(0)) == std::future_status::ready){
        return m_request_result.get();
    }
    return std::nullopt;
}
