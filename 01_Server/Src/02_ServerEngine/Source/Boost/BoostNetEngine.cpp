#include "Boost/BoostNetEngine.h"

BoostNetEngine::BoostNetEngine(UINT32 sessionCount, UINT32 threadCount)
    : m_isRun(false)
    , m_threads(threadCount)
    , m_workGuard(boost::asio::make_work_guard(m_ioContext))
    , m_sessions(sessionCount)
{
    for (UINT32 i = 0; i < sessionCount; ++i)
    {
        BoostSession& session = m_sessions[i].emplace(*this, m_ioContext);
        m_sessionPool.Push(&session);
    }
}

BoostNetEngine::~BoostNetEngine()
{
}

void BoostNetEngine::Start()
{
    m_isRun.store(true);
    OnStart();

    for (UINT32 i = 0; i < m_threads.size(); ++i)
    {
        std::string threadName = "BoostNetIO_" + std::to_string(i);
        m_threads[i].Start(threadName, [this]()
        {
            this->m_ioContext.run();
        });
    }
}

void BoostNetEngine::Stop()
{
    bool expected = true;                                            // 기대값
    bool desired = false;                                            // 변경값
    if (false == m_isRun.compare_exchange_strong(expected, desired)) // 변경되었다면 true 반환
        return;

    OnStop();

    m_ioContext.stop();
    m_workGuard.reset();

    for (Thread& thread : m_threads)
    {
        thread.Join();
    }
}

void BoostNetEngine::PushSession(BoostSession* session)
{
    m_sessionPool.Push(session);
}

bool BoostNetEngine::TryPopSession(BoostSession*& session)
{
    return m_sessionPool.TryPop(session);
}
