#pragma once
#include "Boost/BoostSession.h"
#include "DataStruct/Lock/LockStack.h"
#include "Threading/Thread.h"

class BoostNetEngine
{
    using IoContext = boost::asio::io_context;
    using WorkGuard = boost::asio::executor_work_guard<IoContext::executor_type>;

public:
    BoostNetEngine() = delete;
    BoostNetEngine(UINT32 sessionCount, UINT32 threadCount);
    virtual ~BoostNetEngine();

    BoostNetEngine(const BoostNetEngine&) = delete;
    BoostNetEngine(BoostNetEngine&&) = delete;
    BoostNetEngine& operator=(const BoostNetEngine&) = delete;
    BoostNetEngine& operator=(BoostNetEngine&&) = delete;

    void Start();
    void Stop();

protected:
    virtual void OnStart() = 0;
    virtual void OnStop() = 0;

    IoContext& GetIoContext() { return m_ioContext; }
    bool IsRunning() const { return m_isRun.load(); }
    void PushSession(BoostSession* session);
    bool TryPopSession(BoostSession*& session);

private:
    std::atomic<bool> m_isRun;
    std::vector<Thread> m_threads;
    IoContext m_ioContext;
    WorkGuard m_workGuard;
    std::vector<std::optional<BoostSession>> m_sessions;
    LockStack<BoostSession*> m_sessionPool;
};
