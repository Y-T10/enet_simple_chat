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

ENetPacket* create_packet(
const std::function<void(msgpack::packer<msgpack::sbuffer>&)>& writer){
    msgpack::sbuffer buffer;
    msgpack::packer hoge(buffer);
    writer(hoge);
    return enet_packet_create(
            buffer.data(), buffer.size(),
            ENET_PACKET_FLAG_RELIABLE);
}

ENetPacket* create_stream_packet(
const std::function<void(msgpack::packer<msgpack::sbuffer>&)>& writer){
    msgpack::sbuffer buffer;
    msgpack::packer hoge(buffer);
    writer(hoge);
    return enet_packet_create(
            buffer.data(), buffer.size(),
            ENET_PACKET_FLAG_UNRELIABLE_FRAGMENT);
}