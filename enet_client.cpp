#include"enet_client.hpp"
#include<cassert>

NetClient::NetClient() noexcept
:basic_enet(){}

NetClient::~NetClient(){}

const bool NetClient::request_connection(const ENetAddress& dst, const size_t max_ch, const enet_uint32 data){
    assert(*this);
    //接続要求を送る
    ENetPeer* p = enet_host_connect(m_host, &dst, max_ch, data);
    if(p != NULL){
        p->data = NULL;
        return true;
    }
    return false;
}