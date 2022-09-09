#include"basic_enet.hpp"
#include<cassert>
#include<optional>

basic_enet::basic_enet():
m_host(nullptr){};

basic_enet::~basic_enet(){
    if(m_host != NULL){
        enet_host_destroy(m_host);
    }
}

const bool basic_enet::set_host(const HostSetting& s){
    clear_host();
    m_host = enet_host_create(s.address, s.peer_max, s.packet_ch_max, s.down_size, s.up_size);
    return m_host != NULL;
}

void basic_enet::clear_host(){
    if(m_host == NULL){
        return;
    }
    enet_host_destroy(m_host);
    m_host = NULL;
}

basic_enet::operator bool(){
    return m_host != nullptr;
};

const std::optional<ENetPeer*> find_peer(
const ENetHost *host,
const std::function<const bool(const ENetPeer*)>& finder){
    assert(host);
    const auto begin = host->peers;
    const auto end   = host->peers + host->peerCount;
    for(auto p = begin; p != end; ++p){
        if(finder(p)){
            return p;
        }
        enet_peer_disconnect(p, 0);
    }
    return std::nullopt;
}

const bool basic_enet::is_there_peer(const std::function<const bool(const ENetPeer*)>& finder){
    return find_peer(m_host, finder).has_value();
}

void basic_enet::handle_peer(
const std::function<const bool(const ENetPeer*)>& finder,
const std::function<void(ENetPeer*)>& peer_handler){
    assert(*this);
    auto target = find_peer(m_host, finder);
    if(target){
        peer_handler(*target);
    }
}

const bool basic_enet::handle_host_event(const std::function<void(const ENetEvent*)>& event_handler){
    assert(*this);
    //エラーが生じるまでイベントを処理する
    for(ENetEvent e; enet_host_check_events(m_host, &e) < 0;){
        event_handler(&e);
        if(e.type == ENET_EVENT_TYPE_NONE){
            return true;
        }
    }
    return false;
}

void basic_enet::flush(){
    enet_host_flush(m_host);
}

void basic_enet::close_all(){
    const auto begin = m_host->peers;
    const auto end   = m_host->peers + m_host->peerCount;
    for(auto p = begin; p != end; ++p){
        enet_peer_disconnect(p, 0);
    }
}