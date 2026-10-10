#include "EngineCommon/PacketDispatcher.h"
#include "EngineInterface/INetEngine.h"
#include "EngineInterface/ISession.h"

PacketDispatcher::PacketDispatcher(INetEngine& netEngine)
    : m_netEngine(netEngine)
{
}

std::unique_ptr<PacketBody> PacketDispatcher::CreateBody(UINT16 packetId) const
{
    const std::unordered_map<UINT16, PacketEntry>::const_iterator iter = m_entries.find(packetId);
    if (m_entries.end() == iter)
    {
        return nullptr;
    }

    return iter->second.m_createBody();
}

void PacketDispatcher::EnqueuePacket(Packet&& packet)
{
    m_packetQueue.Push(std::move(packet));
}

UINT32 PacketDispatcher::ProcessPackets()
{
    UINT32 processCount = 0;
    Packet packet;
    while (m_packetQueue.TryPop(packet))
    {
        Dispatch(std::move(packet));
        ++processCount;
    }

    return processCount;
}

void PacketDispatcher::Dispatch(Packet&& packet)
{
    const std::unordered_map<UINT16, PacketEntry>::const_iterator iter = m_entries.find(packet.GetPacketId());
    if (m_entries.end() == iter)
    {
        // TODO : Error Log
        return;
    }

    ISession* session = m_netEngine.FindSession(packet.GetSessionId());
    if (nullptr == session)
    {
        return;
    }

    iter->second.m_handler(*session, *packet.GetBody<PacketBody>());
}