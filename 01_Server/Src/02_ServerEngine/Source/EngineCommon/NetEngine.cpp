#include "EngineCommon/NetEngine.h"

NetEngine::NetEngine(UINT32 threadCount)
    : m_isRun(false)
    , m_threads(threadCount)
{
}

void NetEngine::Start()
{
    m_isRun.store(true);

    // for (UINT32 i = 0; i < m_threads.size(); ++i)
    // {
    //     std::string threadName = "NetEngineIO_" + std::to_string(i);
    //     m_threads[i].Start(threadName, [this]()
    //     {
    //         this->m_ioContext.run();
    //     });
    // }

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

void NetEngine::AddStartEvent(Delegate<void()>::Callback&& event)
{
    m_startDelegator.Subscribe(std::move(event));
}
void NetEngine::AddStopEvent(Delegate<void()>::Callback&& event)
{
    m_stopDelegator.Subscribe(std::move(event));
}