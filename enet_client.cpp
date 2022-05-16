#include"enet_client.hpp"

#include<cassert>

using namespace std;

ENetClient::ENetClient(
const size_t ch_count = 1,
const enet_uint32 down_band_width = 0,
const enet_uint32 up_band_width = 0) noexcept
:m_client(enet_host_create(NULL, 1, ch_count, down_band_width, up_band_width))
,m_communicate_thread(){
    const auto sub_thread = [](
        const size_t ch,
        const enet_uint32 down_width,
        const enet_uint32 up_width){

    };
    m_communicate_thread = thread(sub_thread, ch_count, down_band_width, up_band_width);
    assert(m_client != NULL);
}

ENetClient::~ENetClient(){
    enet_host_destroy(m_client);
}

const bool ENetClient::IsVailed() noexcept{
    if(m_client == NULL){
        return false;
    }
    return true;
}

const bool ENetClient::request_connection(
const std::string& host_name,
const enet_uint16 port
) noexcept{
    if(host_name.empty()){
        return false;
    }
    ENetAddress address;
    enet_address_set_host(&address, host_name.data());
    address.port = port;

    ENetPeer *peer = enet_host_connect(m_client, &address, m_client->channelLimit, NULL);
    if(peer == NULL) {
        return 0;
    }
}