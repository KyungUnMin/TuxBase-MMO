#pragma once
#include "DataStruct/RingBuffer/RingBuffer.h"

class BoostNetEngine;

class BoostSession
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

private:
    Socket m_socket;
    BoostNetEngine& m_netEngine;
    RingBuffer m_recvBuffer;
    RingBuffer m_sendBuffer;
};