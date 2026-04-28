#pragma once
#include "Common/ISession.h"
#include "DataStruct/RingBuffer/RingBuffer.h"

class BoostNetEngineServer;

class BoostSession : public ISession
{
    static constexpr UINT32 kBufferSize = 1024 * 16;

    using Socket = boost::asio::ip::tcp::socket;
    using IoContext = boost::asio::io_context;

public:
    BoostSession() = delete;
    BoostSession(BoostNetEngineServer& netEngine, IoContext& ioContext);
    ~BoostSession() override = default;

    BoostSession(const BoostSession&) = delete;
    BoostSession(BoostSession&&) = delete;
    BoostSession& operator=(const BoostSession&) = delete;
    BoostSession& operator=(BoostSession&&) = delete;

public:
    void Start();
    Socket& GetSocket() { return m_socket; }

private:
    Socket m_socket;
    BoostNetEngineServer& m_netEngine;
    RingBuffer m_recvBuffer;
    RingBuffer m_sendBuffer;
};