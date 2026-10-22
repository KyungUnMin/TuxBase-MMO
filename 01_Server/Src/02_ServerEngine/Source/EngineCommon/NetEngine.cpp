#include "EngineCommon/NetEngine.h"
#include "Verify/Assert.h"

NetEngine::NetEngine(UINT32 ioThreadCount, UINT32 sessionCount, UPtr<SessionFactory>&& sessionFactory)
    : m_isRun(false)
    , m_threads(ioThreadCount)
    , m_sessionFactory(std::move(sessionFactory))
{
    ASSERT(0 < ioThreadCount, "IoThreadCount is zero");
    ASSERT(0 < sessionCount, "SessionCount is zero");
    ASSERT(nullptr != sessionFactory, "sessionFactory is null");
    m_sessions.resize(sessionCount, nullptr);
}

void NetEngine::Start()
{
    ASSERT(0 < m_threads.size(), "threadCount is zero");
    for (UINT32 i = 0; i < m_threads.size(); ++i)
    {
        std::string threadName = "NetEngineIO_" + std::to_string(i);
        m_threads[i].Start(threadName, std::bind(&NetEngine::RunThreadWorker, this));
    }

    ASSERT(0 < m_sessions.size(), "SessionCount is zero");
    ASSERT(nullptr != m_sessionFactory, "sessionFactory is null");
    for (SPtr<ISession>& session : m_sessions)
    {
        session = m_sessionFactory->Create();
        if (nullptr == session)
        {
            CRASH("Session is null");
            return;
        }
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
}

bool NetEngine::CreateSessions(UINT32 sessionCount, UPtr<SessionFactory>&& sessionFactory)
{
    ASSERT(0 < sessionCount, "sessionCount is zero");
    ASSERT(nullptr != sessionFactory, "Session factory is null");
    m_sessions.resize(sessionCount, nullptr);
    for (SPtr<ISession>& session : m_sessions)
    {
        session = sessionFactory->Create();
        if (nullptr == session)
        {
            CRASH("Session is null");
            return false;
        }
    }
    return true;
}

void NetEngine::AddStartEvent(Delegate<void()>::Callback&& event)
{
    m_startDelegator.Subscribe(std::move(event));
}
void NetEngine::AddStopEvent(Delegate<void()>::Callback&& event)
{
    m_stopDelegator.Subscribe(std::move(event));
}