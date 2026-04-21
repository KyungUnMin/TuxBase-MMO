#pragma once
#include "Common/ISession.h"
#include "DataStruct/RingBuffer/RingBuffer.h"

class BoostSession : public ISession
{
    static constexpr UINT32 kBufferSize = 1024 * 16;

    using Socket = boost_asio::ip::tcp::socket;
    using IoContext = boost::asio::io_context;
    class BoostNetEngine;

public:
    BoostSession() = delete;
    BoostSession(BoostNetEngine& netEngine, IoContext& ioContext);
    ~BoostSession() override = default;

    BoostSession(const BoostSession&) = delete;
    BoostSession(BoostSession&&) = delete;
    BoostSession& operator=(const BoostSession&) = delete;
    BoostSession& operator=(BoostSession&&) = delete;

public:
    void Start();

private:
    Socket m_socket;
    BoostNetEngine& m_netEngine;
    RingBuffer m_recvBuffer;
    RingBuffer m_sendBuffer;
};