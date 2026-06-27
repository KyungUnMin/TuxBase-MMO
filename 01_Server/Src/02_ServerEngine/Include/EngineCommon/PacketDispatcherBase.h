#pragma once
#include "EngineCommon/Packet.h"
#include "DataStruct/LockBase/LockQueue.h"
#include "DataStruct/RingBuffer/RingBufferReader.h"

class PacketDispatcherBase
{
    class INetEngine;
    class ISession;

public:
    PacketDispatcherBase() = delete;
    virtual ~PacketDispatcherBase() = default;

    PacketDispatcherBase(const PacketDispatcherBase&) = delete;
    PacketDispatcherBase(PacketDispatcherBase&&) = delete;
    PacketDispatcherBase& operator=(const PacketDispatcherBase&) = delete;
    PacketDispatcherBase& operator=(PacketDispatcherBase&&) = delete;

    void EnqueuePacket(UINT64 sessionId, RingBufferReader&& dataReader, const PacketHeader& packetHeader);

protected:
    virtual void Dispatch(ISession& session, Packet&& packet) = 0;

private:
    LockQueue<Packet> m_packetQueue;
};