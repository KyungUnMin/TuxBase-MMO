#pragma once
#include "DataStruct/RecvBuffer.h"
#include "EngineCommon/Packet.h"

class PacketSerializer
{
    using DeserialzeHandler = std::function<UPtr<PacketBody>(UINT16)>;
    using Stl_HashMap = std::unordered_map<UINT16, DeserialzeHandler>;
    using Iterator = Stl_HashMap::iterator;

public:
    PacketSerializer() = default;
    ~PacketSerializer() = default;
    PacketSerializer(const PacketSerializer&) = delete;
    PacketSerializer(PacketSerializer&&) = delete;
    PacketSerializer& operator=(const PacketSerializer&) = delete;
    PacketSerializer& operator=(PacketSerializer&&) = delete;

    void RegisterDeserialzer(UINT16 packetId, DeserialzeHandler&& packerDeserializeHandler)
    {
        ASSERT(nullptr != packerDeserializeHandler, "Handler is null");
        ASSERT(false == m_packerDeserializeHandlers.contains(packetId), "Packet id(%d) handler is already registered", packetId);
        m_packerDeserializeHandlers[packetId] = std::move(packerDeserializeHandler);
    }

    UPtr<Packet> DeserializePacket(const RecvBuffer& recvBuffer);

private:
    static PacketHeader PeekRecvPacketHeader(const RecvBuffer& recvBuffer);

private:
    Stl_HashMap m_packerDeserializeHandlers;
};
