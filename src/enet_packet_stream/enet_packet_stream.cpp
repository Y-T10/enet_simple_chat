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

PacketUnpacker::PacketUnpacker(ENetPacket* packet) noexcept
:m_src(packet)
,m_offset(0){};

PacketUnpacker::operator bool(){
    return m_offset < m_src->dataLength;
}

const msgpack::object_handle PacketUnpacker::get_handler(){
    const char *data = (const char*)(m_src->data);
    const size_t data_size = m_src->dataLength;
    return msgpack::unpack(data, data_size, m_offset);
}