#include "EngineCommon/NetEngine.h"

NetEngine::NetEngine(UINT32 ioThreadCount, UINT32 sessionCount, UPtr<SessionFactory>&& sessionFactory)
    : m_isRun(false)
    , m_threads(ioThreadCount)
    , m_usingSessions(sessionCount)
{
    ASSERT(0 < ioThreadCount, "IoThreadCount is zero");

    ASSERT(0 < sessionCount, "SessionCount is zero");
    ASSERT(nullptr != sessionFactory, "sessionFactory is null");
    m_sessionPool.Reserve(sessionCount);
    for (UINT32 i = 0; i < sessionCount; ++i)
    {
        SPtr<ISession> session = sessionFactory->Create(i);
        if (nullptr == session)
        {
            // CRASH("Session is null");
            return;
        }

        m_sessionPool.Push(session);
    }
}

void NetEngine::Start()
{
    ASSERT(0 < m_threads.size(), "threadCount is zero");
    for (UINT32 i = 0; i < m_threads.size(); ++i)
    {
        std::string threadName = "NetEngineIO_" + std::to_string(i);
        m_threads[i].Start(threadName, std::bind(&NetEngine::RunThreadWorker, this));
    }

    m_isRun.store(true);
    m_startDelegator.Invoke();
}

void NetEngine::Stop()
{
    bool expected = true;                                            // 기대값
    bool desired = false;                                            // 변경값
    if (false == m_isRun.compare_exchange_strong(expected, desired)) // 변경되었다면 true 반환
        return;

    m_stopDelegator.Invoke();

    for (Thread& thread : m_threads)
    {
        thread.Join();
    }
}

void NetEngine::AddStartEvent(Delegate<void()>::Callback&& event)
{
    m_startDelegator.Subscribe(std::move(event));
}
void NetEngine::AddStopEvent(Delegate<void()>::Callback&& event)
{
    m_stopDelegator.Subscribe(std::move(event));
}

SPtr<ISession> NetEngine::TakeSession()
{
    SPtr<ISession> session = nullptr;
    return m_sessionPool.TryPop(session) ? session : nullptr;
}

void NetEngine::ReleaseSession(SPtr<ISession>&& session)
{
    m_sessionPool.Push(session);
    session = nullptr;
}