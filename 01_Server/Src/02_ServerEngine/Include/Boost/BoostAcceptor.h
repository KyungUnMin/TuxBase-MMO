#pragma once

class BoostNetEngineServer;
class BoostSession;

class BoostAcceptor
{
    using IoContext = boost::asio::io_context;
    using ErrorCode = boost::system::error_code;
    using RawAcceptor = boost::asio::ip::tcp::acceptor;
    using BoostTimer = boost::asio::steady_timer;

public:
    BoostAcceptor() = delete;
    BoostAcceptor(UINT16 portNum, BoostNetEngineServer& netEngine, IoContext& ioContext);
    ~BoostAcceptor() = default;

    BoostAcceptor(const BoostAcceptor&) = delete;
    BoostAcceptor(BoostAcceptor&&) = delete;
    BoostAcceptor& operator=(const BoostAcceptor&) = delete;
    BoostAcceptor& operator=(BoostAcceptor&&) = delete;

    void StartListening();
    void StopListening();

private:
    void AcceptSession();
    void CompleteAcception(SPtr<BoostSession> session, const ErrorCode& errorCode);
    void RetryAcception();

private:
    UINT16 m_portNum;
    RawAcceptor m_rawAcceptor;
    BoostNetEngineServer& m_netEngine;
    BoostTimer m_acceptRetryTimer;
};