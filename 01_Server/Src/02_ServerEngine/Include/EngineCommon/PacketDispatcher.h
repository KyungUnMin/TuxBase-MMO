#pragma once
#include "EngineCommon/Packet.h"

class ISession;

/*-------------------------------------
            PacketDispatcher
-------------------------------------*/

class PacketDispatcher
{
    using PacketHandler = std::function<bool(const SPtr<ISession>&, const UPtr<PacketBody>&)>;
    using Stl_HashMap = std::unordered_map<UINT16, PacketHandler>;
    using Iterator = Stl_HashMap::iterator;

public:
    PacketDispatcher() = default;
    ~PacketDispatcher() = default;
    PacketDispatcher(const PacketDispatcher&) = delete;
    PacketDispatcher(PacketDispatcher&&) = delete;
    PacketDispatcher& operator=(const PacketDispatcher&) = delete;
    PacketDispatcher& operator=(PacketDispatcher&&) = delete;

    void RegisterPacketHandler(UINT16 packetId, PacketHandler&& packetHandler);
    bool ProcessPacket(const SPtr<ISession>& session, const UPtr<Packet>& packet);

private:
    Stl_HashMap m_packetHandlers;
};
