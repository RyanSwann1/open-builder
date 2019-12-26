#include "server.h"

#include "../server_config.h"
#include <SFML/System/Clock.hpp>
#include <common/debug.h>
#include <thread>

#include "../world/terrain_generation.h"

Server::Server()
    : NetworkHost("Server")
{
    // Create "spawn"
    m_spawn = &m_chunkManager.addChunk({0, 0, 0});
    makeFlatTerrain(m_spawn);
}

void Server::sendChunk(peer_id_t peerId, const Chunk &chunk)
{
    Peer *peer = getPeer(peerId);
    assert(peer);
    if (peer) {
        // Create the chunk-data packet
        sf::Packet packet;
        packet << ClientCommand::ChunkData << chunk.getPosition().x
               << chunk.getPosition().y << chunk.getPosition().z;

        for (const auto &block : chunk.blocks) {
            packet << block;
        }
        packet.append(chunk.blocks.data(),
                      chunk.blocks.size() * sizeof(chunk.blocks[0]));

        // Send chunk data to client
        sendToPeer(peer->peer, packet, 1, ENET_PACKET_FLAG_RELIABLE);
    }
}

void Server::onPeerConnect(ENetPeer *peer)
{
    if (!isFull()) {
        ++m_peerID;
        peer_id_t id = static_cast<peer_id_t>(m_peerID);

        // Send client back their id
        sf::Packet packet;
        packet << ClientCommand::PeerId << id;
        NetworkHost::sendToPeer(peer, packet, 0, ENET_PACKET_FLAG_RELIABLE);

        // Broadcast the connection event
        sf::Packet announcement;
        announcement << ClientCommand::PlayerJoin << id;
        broadcastToPeers(announcement, 0, ENET_PACKET_FLAG_RELIABLE);

        addPeer(peer, id);
    }
}

void Server::onPeerDisconnect(ENetPeer *peer)
{
    removePeer(peer->connectID);
}

void Server::onPeerTimeout(ENetPeer *peer)
{
    removePeer(peer->connectID);
}

void Server::onCommandRecieve(ENetPeer *peer, sf::Packet &packet,
                              command_t command)
{
    switch (static_cast<ServerCommand>(command)) {
        case ServerCommand::PlayerPosition:
            handleCommandPlayerPosition(packet);
            break;

        case ServerCommand::Disconnect:
            handleCommandDisconnect(packet);
            break;

        case ServerCommand::ChunkRequest:
            handleCommandChunkRequest(packet);
            break;
    }
}

void Server::handleCommandDisconnect(sf::Packet &packet)
{
    peer_id_t id;
    packet >> id;
    auto iter =
        std::find_if(m_peers.begin(), m_peers.end(), [id](const auto &peer) {
           return peer->peer->connectID == id;
        });
    assert(iter != m_peers.end());
    if (iter != m_peers.end()) {
        m_peers.erase(iter);

        // Broadcast the disconnection event
        sf::Packet announcement;
        announcement << ClientCommand::PlayerLeave << id;
        broadcastToPeers(announcement, 0, ENET_PACKET_FLAG_RELIABLE);
    }
}

void Server::handleCommandPlayerPosition(sf::Packet &packet)
{
    peer_id_t id;
    packet >> id;
    auto iter =
        std::find_if(m_peers.begin(), m_peers.end(), [id](const auto &peer) {
            return peer->peer->connectID == id;
        });
    assert(iter != m_peers.end());
    if (iter != m_peers.end())
    {
        packet >> iter->get()->position.x >> iter->get()->position.y >> iter->get()->position.z;
    }
}

void Server::handleCommandChunkRequest(sf::Packet &packet)
{
    peer_id_t id;
    ChunkPosition position;
    packet >> id >> position.x >> position.y >> position.z;

    m_chunkRequests.emplace(position, id);
}

void Server::sendPackets()
{
    // Player positions
    {
        sf::Packet packet;
        u16 count = NetworkHost::getConnectedPeerCount();
        packet << ClientCommand::Snapshot << count;
        for (const auto &peer : m_peers) {
            packet << peer->peer->connectID << peer->position.x
                   << peer->position.y << peer->position.z;
        }

        broadcastToPeers(packet, 0, 0);
    }

    // Chunks
    {
        // Add 1 per tick to the queue for now
        if (!m_chunkRequests.empty()) {

            auto cr = m_chunkRequests.front();
            m_chunkRequests.pop();

            Chunk &chunk = m_chunkManager.addChunk(cr.position);
            makeFlatTerrain(&chunk);

            // Create the chunk-data packet
            sendChunk(cr.peer, chunk);
        }
    }
}

void Server::addPeer(ENetPeer *peer, peer_id_t id)
{
    LOGVAR("Server", "New Peer, Peer Id:", (int)id);
    m_peers.emplace_back(std::make_unique<Peer>(peer, id));
}

void Server::removePeer(u32 connectionId)
{
    auto iter = std::find_if(m_peers.begin(), m_peers.end(), [connectionId]
    (const auto &peer) { return peer->peer->connectID == connectionId; });

    assert(iter != m_peers.end());
    if (iter != m_peers.end()) {
        LOGVAR("Server", "Peer exited, Peer Id:", iter->get()->peer->connectID);
        m_peers.erase(iter);
        
    }
}

bool Server::isFull() const
{
    return m_peers.size() == NetworkHost::getMaxConnections();
}

Peer *Server::getPeer(peer_id_t peerID) const
{
    auto cIter = std::find_if(m_peers.cbegin(), m_peers.cend(),
                              [peerID](const auto &peer) {
                                  return peer->peer->connectID == peerID;
                              });
    return (cIter != m_peers.cend() ? cIter->get() : nullptr);
}
