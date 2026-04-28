#pragma once
#include "Common/INetEngineServer.h"
#include "Boost/BoostSession.h"
#include "DataStruct/Lock/LockStack.h"
#include "Threading/Thread.h"

class BoostNetEngineServer : public INetEngineServer
{
    using IoContext = boost::asio::io_context;
    using Acceptor = boost::asio::ip::tcp::acceptor;
    using ErrorCode = boost::system::error_code;
    using WorkGuard = boost::asio::executor_work_guard<IoContext::executor_type>;

public:
    static constexpr UINT32 kMaxSessionCount = 1024;

    explicit BoostNetEngineServer(UINT32 threadCount = 1);
    ~BoostNetEngineServer() override;

    BoostNetEngineServer(const BoostNetEngineServer&) = delete;
    BoostNetEngineServer(BoostNetEngineServer&&) = delete;
    BoostNetEngineServer& operator=(const BoostNetEngineServer&) = delete;
    BoostNetEngineServer& operator=(BoostNetEngineServer&&) = delete;

    void Start(UINT16 port) override;
    void Stop() override;

private:
    void Listen(UINT16 port);
    void Accept();
    void RetryAccept();
    void CompleteAccept(BoostSession* session, const ErrorCode& errorCode);
    void Update();

private:
    std::atomic<bool> m_isRun;
    IoContext m_ioContext;
    WorkGuard m_workGuard;
    Acceptor m_accepter;
    boost::asio::steady_timer m_acceptRetryTimer;
    std::array<std::optional<BoostSession>, kMaxSessionCount> m_sessions;
    LockStack<BoostSession*, kMaxSessionCount> m_sessionPool;
    std::vector<Thread> m_threads;
};