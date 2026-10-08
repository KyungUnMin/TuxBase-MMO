#pragma once
#include "DataStruct/RecvBuffer.h"
#include "EngineCommon/Packet.h"
#include "DataStruct/SendBuffer.h"
#include "EngineInterface/ISession.h"

class BoostNetEngine;

class BoostSession : public ISession
{
    using Socket = boost::asio::ip::tcp::socket;
    using IoContext = boost::asio::io_context;
    using ErrorCode = boost::system::error_code;

public:
    static constexpr UINT32 kRecvBufferSize = PacketHeader::kMaxPacketSize * 2;
    static constexpr UINT32 kMaxSendQueueSize = 1024;

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

    bool Send(std::shared_ptr<const SendBuffer> sendBuffer) override;

private:
    void FlushSendQueue();
    void CompleteSend(const ErrorCode& errorCode);

private:
    Socket m_socket;
    BoostNetEngine& m_netEngine;
    RecvBuffer m_recvBuffer;

    std::mutex m_sendMutex;
    std::vector<std::shared_ptr<const SendBuffer>> m_sendQueue;
    bool m_isSending;
    std::vector<std::shared_ptr<const SendBuffer>> m_sendingBuffers;
};