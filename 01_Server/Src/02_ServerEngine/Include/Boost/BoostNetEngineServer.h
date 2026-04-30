#pragma once
#include "Boost/BoostNetEngine.h"

class BoostNetEngineServer : public BoostNetEngine
{
    using Acceptor = boost::asio::ip::tcp::acceptor;
    using ErrorCode = boost::system::error_code;
    using BoostTimer = boost::asio::steady_timer;

public:
    BoostNetEngineServer() = delete;
    BoostNetEngineServer(UINT16 port, UINT32 sessionCount, UINT32 threadCount = 1);
    ~BoostNetEngineServer() override;

    BoostNetEngineServer(const BoostNetEngineServer&) = delete;
    BoostNetEngineServer(BoostNetEngineServer&&) = delete;
    BoostNetEngineServer& operator=(const BoostNetEngineServer&) = delete;
    BoostNetEngineServer& operator=(BoostNetEngineServer&&) = delete;

    void OnStart() override;
    void OnStop() override;

private:
    void Accept();
    void RetryAccept();
    void CompleteAccept(BoostSession* session, const ErrorCode& errorCode);

private:
    const UINT16 m_port;
    Acceptor m_accepter;
    BoostTimer m_acceptRetryTimer;
};