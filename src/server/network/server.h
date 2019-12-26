#pragma once

#include <SFML/System/Time.hpp>
#include <array>
#include <common/network/net_host.h>
#include <common/world/chunk_manager.h>
#include <queue>
#include <unordered_map>

struct ServerConfig;

struct ServerEntity {
    float x = 0, y = 0, z = 0;
    bool active = false;
};

class Peer {
  public:
    Peer(ENetPeer *peer, peer_id_t ID)
        : peer(peer)
    {
        peer->connectID = ID;
    }


    ENetPeer *peer = nullptr;
    sf::Vector3f position;
};

// struct Peer {
//    u32 id = 0;
//    ENetPeer *peer = nullptr;
//    bool isActive = false;
//};

struct ChunkRequest {
    ChunkRequest(ChunkPosition &chunkPosition, peer_id_t peerId)
        : position(chunkPosition)
        , peer(peerId)
    {
    }

    ChunkPosition position;
    peer_id_t peer;
};

class Server final : public NetworkHost {
  public:
    Server();

    void sendPackets();

  private:
    void sendChunk(peer_id_t peerId, const Chunk &chunk);

    void onPeerConnect(ENetPeer *peer) override;
    void onPeerDisconnect(ENetPeer *peer) override;
    void onPeerTimeout(ENetPeer *peer) override;
    void onCommandRecieve(ENetPeer *peer, sf::Packet &packet,
                          command_t command) override;

    void handleCommandDisconnect(sf::Packet &packet);
    void handleCommandPlayerPosition(sf::Packet &packet);
    void handleCommandChunkRequest(sf::Packet &packet);

    void addPeer(ENetPeer *peer, peer_id_t id);
    void removePeer(u32 connectionId);

    std::array<ServerEntity, 512> m_gameObjects;
    std::vector<std::unique_ptr<Peer>> m_peers;
    int m_peerID = 0;
    ChunkManager m_chunkManager;
    Chunk *m_spawn;
    std::queue<ChunkRequest> m_chunkRequests;

    bool m_isRunning = true;
    bool isFull() const;
    Peer *getPeer(peer_id_t peerID) const;
};