#pragma once
#include "DataStruct/RingBuffer/RingBuffer.h"
#include "EngineCommon/PacketSerializer.h"
#include "EngineInterface/ISession.h"

class BoostNetEngine;

class BoostSession : public ISession
{
    using Socket = boost::asio::ip::tcp::socket;
    using IoContext = boost::asio::io_context;

public:
    BoostSession() = delete;
    BoostSession(BoostNetEngine& netEngine, IoContext& ioContext);
    ~BoostSession() = default;

    BoostSession(const BoostSession&) = delete;
    BoostSession(BoostSession&&) = delete;
    BoostSession& operator=(const BoostSession&) = delete;
    BoostSession& operator=(BoostSession&&) = delete;

public:
    void Start();
    void CloseSocket();
    Socket& GetSocket() { return m_socket; }

    template <typename TMessage>
    bool SendPacket(UINT16 packetId, const TMessage& message)
    {
        return PacketSerializer::Write(m_sendBuffer, packetId, message);
    }

    bool PeekPacketHeader(PacketHeader& outHeader)
    {
        return PacketSerializer::PeekHeader(m_recvBuffer, outHeader);
    }

    template <typename TMessage>
    bool ReadPacket(const PacketHeader& header, TMessage& outMessage)
    {
        return PacketSerializer::Read(m_recvBuffer, header, outMessage);
    }

    template <typename TMessage>
    TMessage* ReadPacket(const PacketHeader& header, google::protobuf::Arena& arena)
    {
        return PacketSerializer::Read<TMessage>(m_recvBuffer, header, arena);
    }

private:
    Socket m_socket;
    BoostNetEngine& m_netEngine;
    RingBuffer m_recvBuffer;
    RingBuffer m_sendBuffer;
};