#include "EngineCommon/PacketDispatcher.h"
#include "EngineInterface/INetEngine.h"
#include "EngineInterface/ISession.h"

void PacketDispatcher::RegisterPacketHandler(UINT16 packetId, PacketHandler&& packetHandler)
{
    ASSERT(nullptr != packetHandler, "Handler is null");
    ASSERT(false == m_packetHandlers.contains(packetId), "Packet id(%d) handler is already registered", packetId);
    m_packetHandlers[packetId] = std::move(packetHandler);
}

bool PacketDispatcher::ProcessPacket(const SPtr<ISession>& session, const UPtr<Packet>& packet)
{
    if (nullptr == session || nullptr == packet)
        return false;

    PacketHeader packetHeader = packet->GetHeader();
    Iterator findIter = m_packetHandlers.find(packetHeader.m_id);
    ASSERT((findIter != m_packetHandlers.end()), "Packet id(%d) handler is not exists", packetId);
    const PacketHandler& packetHandler = findIter->second;
    return packetHandler(session, packet->GetBody());
}
