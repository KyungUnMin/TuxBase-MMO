#include "Boost/BoostNetEngine.h"
#include <thread>

BoostNetEngine::BoostNetEngine()
    : m_isRun(false)
    , m_accepter(m_ioContext)
{
    m_sessionPool.Reserve(kMaxSessionCount);
}

BoostNetEngine::~BoostNetEngine()
{
    Stop();
}

void BoostNetEngine::Start(const UINT16 port)
{
    CreateSessions();
    m_isRun.store(true);

    Listen(port);
    Update();
}

void BoostNetEngine::CreateSessions()
{
    for (UINT32 i = 0; i < kMaxSessionCount; ++i)
    {
        BoostSession& session = m_sessions[i].emplace(*this, m_ioContext);
        m_sessionPool.Push(&session);
    }
}

void BoostNetEngine::Listen(const UINT16 port)
{
    m_accepter.open(boost::asio::ip::tcp::v4());
    m_accepter.bind(boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port));
    m_accepter.listen();
    m_acceptThread.Start("ListenWorkerThread", [this]()
    {
        ListenWorkerThreadFunc();
    });
}

void BoostNetEngine::ListenWorkerThreadFunc()
{
    while (m_isRun.load())
    {
        BoostSession* session = nullptr;
        while (nullptr == session && false == m_sessionPool.TryPop(session))
        {
            if (!m_isRun.load())
                return;

            // TODO : LOG_ERROR("Session pool is empty.");
            std::this_thread::yield();
        }

        ErrorCode errorCode;
        m_accepter.accept(session->GetSocket(), errorCode);
        if (errorCode)
        {
            m_sessionPool.Push(session);

            // Stop()에 의한 정상 종료
            if (errorCode == boost::asio::error::operation_aborted)
                break;

            // LOG_ERROR("Accept error: {}", errorCode.message());
            continue;
        }

        session->Start();
    }
}

void BoostNetEngine::Update()
{
    // TODO : 스레드 풀을 만들어서 ioContext.run()을 실행
    m_ioContext.run(); // 일단 오늘은 빌드 되는지만 보자
}

void BoostNetEngine::Stop()
{
    bool expected = true;                                            // 기대값
    bool desired = false;                                            // 변경값
    if (false == m_isRun.compare_exchange_strong(expected, desired)) // 변경되었다면 true 반환
        return;

    m_accepter.close();
    m_acceptThread.Join();
}