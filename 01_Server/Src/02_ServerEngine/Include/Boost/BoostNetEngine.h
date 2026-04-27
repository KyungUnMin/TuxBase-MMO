#pragma once
#include "Common/INetEngine.h"
#include "Boost/BoostSession.h"
#include "DataStruct/Lock/LockStack.h"
#include "Threading/Thread.h"

class BoostNetEngine : public INetEngine
{
    using IoContext = boost::asio::io_context;
    using Acceptor = boost::asio::ip::tcp::acceptor;
    using ErrorCode = boost::system::error_code;
    using WorkGuard = boost::asio::executor_work_guard<IoContext::executor_type>;

public:
    static constexpr UINT32 kMaxSessionCount = 1024;

    BoostNetEngine() = delete;
    BoostNetEngine(UINT32 threadCount = 1, UINT32 sessionCount = kMaxSessionCount);
    ~BoostNetEngine() override;

    BoostNetEngine(const BoostNetEngine&) = delete;
    BoostNetEngine(BoostNetEngine&&) = delete;
    BoostNetEngine& operator=(const BoostNetEngine&) = delete;
    BoostNetEngine& operator=(BoostNetEngine&&) = delete;

    void Start(const UINT16 port);
    void Stop();

private:
    void Listen(const UINT16 port);
    void Accept();
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