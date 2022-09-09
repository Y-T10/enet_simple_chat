#include"enet_host.hpp"
#include<cassert>

NetHost::NetHost() noexcept
:basic_enet(){
}

NetHost::~NetHost(){}

void NetHost::broadcast(const size_t ch, ENetPacket* packet){
    assert(*this);
    enet_host_broadcast(m_host, ch, packet);
}