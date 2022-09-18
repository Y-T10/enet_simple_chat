#include"enet_packet_stream.hpp"

PacketStream::PacketStream() noexcept
:m_buffer(){}

PacketStream::~PacketStream(){
    m_buffer.clear();
}

ENetPacket* PacketStream::realtime_packet() noexcept{
    return enet_packet_create(
            m_buffer.data(), m_buffer.size(),
            ENET_PACKET_FLAG_UNRELIABLE_FRAGMENT);
}

ENetPacket* PacketStream::packet() noexcept{            
    return enet_packet_create(
            m_buffer.data(), m_buffer.size(),
            ENET_PACKET_FLAG_RELIABLE);
};