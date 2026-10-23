#pragma once
#include "EngineCommon/NetEngine.h"
#include "BoostAcceptor.h"

class BoostSession;

/*-------------------------------------
  BoostNetEngine
-------------------------------------*/
class BoostNetEngine : public NetEngine
{
    using IoContext = boost::asio::io_context;
    using WorkGuard = boost::asio::executor_work_guard<IoContext::executor_type>;
    using BoostSessionFactory = Factory<ISession, BoostSession, SPtr<BoostNetEngine>, IoContext&>;

public:
    BoostNetEngine() = delete;
    BoostNetEngine(UINT32 ioThreadCount, UINT32 sessionCount);
    ~BoostNetEngine() override;

    BoostNetEngine(const BoostNetEngine&) = delete;
    BoostNetEngine(BoostNetEngine&&) = delete;
    BoostNetEngine& operator=(const BoostNetEngine&) = delete;
    BoostNetEngine& operator=(BoostNetEngine&&) = delete;

    SPtr<BoostSession> TakeBoostSession();

protected:
    IoContext& GetIoContext() { return m_ioContext; }

private:
    void RunThreadWorker() final;
    void StopEngineEvent();

private:
    IoContext m_ioContext;
    WorkGuard m_workGuard;
};

/*-------------------------------------
  BoostNetEngineServer
-------------------------------------*/
class BoostNetEngineServer : public BoostNetEngine
{
public:
    BoostNetEngineServer() = delete;
    BoostNetEngineServer(UINT16 portNum, UINT32 ioThreadCount, UINT32 sessionCount);
    ~BoostNetEngineServer() override;

    BoostNetEngineServer(const BoostNetEngineServer&) = delete;
    BoostNetEngineServer(BoostNetEngineServer&&) = delete;
    BoostNetEngineServer& operator=(const BoostNetEngineServer&) = delete;
    BoostNetEngineServer& operator=(BoostNetEngineServer&&) = delete;

private:
    void StartEngineEvent();
    void StopEngineEvent();

private:
    BoostAcceptor m_acceptor;
};

/*-------------------------------------
  BoostNetEngineClient
-------------------------------------*/

class BoostNetEngineClient : public BoostNetEngine
{
    using Endpoint = boost::asio::ip::tcp::endpoint;
    using ErrorCode = boost::system::error_code;
    using BoostTimer = boost::asio::steady_timer;

public:
    BoostNetEngineClient() = delete;
    BoostNetEngineClient(std::string_view host, UINT16 port, UINT32 sessionCount = 1, UINT32 threadCount = 1);
    ~BoostNetEngineClient() override;

    BoostNetEngineClient(const BoostNetEngineClient&) = delete;
    BoostNetEngineClient(BoostNetEngineClient&&) = delete;
    BoostNetEngineClient& operator=(const BoostNetEngineClient&) = delete;
    BoostNetEngineClient& operator=(BoostNetEngineClient&&) = delete;

private:
    void Connect(BoostSession* session);
    void CompleteConnect(BoostSession* session, const ErrorCode& errorCode);
    void RetryConnect(BoostSession* session);

private:
    Endpoint m_endpoint;
    BoostTimer m_connectRetryTimer;
};