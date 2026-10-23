#include "Boost/BoostNetEngine.h"
#include "Boost/BoostSession.h"

BoostNetEngine::BoostNetEngine(UINT32 ioThreadCount, UINT32 sessionCount)
    : NetEngine(ioThreadCount, sessionCount, std::make_unique<BoostSessionFactory>(*this, m_ioContext))
    , m_workGuard(boost::asio::make_work_guard(m_ioContext))
{
}

BoostNetEngine::~BoostNetEngine()
{
}

void BoostNetEngine::RunThreadWorker()
{
    while (IsRunning())
    {
        m_ioContext.run();
    }
}

// void BoostNetEngine::Stop()
// {
//     bool expected = true;                                            // 기대값
//     bool desired = false;                                            // 변경값
//     if (false == m_isRun.compare_exchange_strong(expected, desired)) // 변경되었다면 true 반환
//         return;

//     OnStop();

//     m_ioContext.stop();
//     m_workGuard.reset();

//     for (Thread& thread : m_threads)
//     {
//         thread.Join();
//     }
// }