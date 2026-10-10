#pragma once
#include "EngineCommon/Packet.h"
#include "DataStruct/LockQueue.h"
#include <concepts>
#include <unordered_map>

class INetEngine;
class ISession;

class PacketDispatcher
{
public:
    PacketDispatcher() = delete;
    explicit PacketDispatcher(INetEngine& netEngine);
    ~PacketDispatcher() = default;

    PacketDispatcher(const PacketDispatcher&) = delete;
    PacketDispatcher(PacketDispatcher&&) = delete;
    PacketDispatcher& operator=(const PacketDispatcher&) = delete;
    PacketDispatcher& operator=(PacketDispatcher&&) = delete;

    template <std::derived_from<PacketBody> TMessage>
    void Register(UINT16 packetId, std::function<void(ISession&, const TMessage&)> handler)
    {
        ASSERT(nullptr != handler, "Handler is null");
        ASSERT(!m_entries.contains(packetId), "Packet id is already registered");

        PacketEntry entry;
        entry.m_createBody = []() -> std::unique_ptr<PacketBody>
        {
            return std::make_unique<TMessage>();
        };
        entry.m_handler = [handler = std::move(handler)](ISession& session, const PacketBody& body)
        {
            handler(session, static_cast<const TMessage&>(body));
        };

        m_entries.emplace(packetId, std::move(entry));
    }

    std::unique_ptr<PacketBody> CreateBody(UINT16 packetId) const;
    void EnqueuePacket(Packet&& packet);
    UINT32 ProcessPackets();

private:
    struct PacketEntry
    {
        std::function<std::unique_ptr<PacketBody>()> m_createBody;
        std::function<void(ISession&, const PacketBody&)> m_handler;
    };

    void Dispatch(Packet&& packet);

private:
    INetEngine& m_netEngine;
    std::unordered_map<UINT16, PacketEntry> m_entries;
    LockQueue<Packet> m_packetQueue;
};