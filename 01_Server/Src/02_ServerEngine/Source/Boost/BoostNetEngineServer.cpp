#include "Boost/BoostNetEngineServer.h"

BoostNetEngineServer::BoostNetEngineServer(UINT32 threadCount /*= 1*/)
    : m_isRun(false)
    , m_accepter(m_ioContext)
    , m_workGuard(boost::asio::make_work_guard(m_ioContext))
    , m_acceptRetryTimer(m_ioContext)
    , m_threads(threadCount)
{
    for (UINT32 i = 0; i < kMaxSessionCount; ++i)
    {
        BoostSession& session = m_sessions[i].emplace(*this, m_ioContext);
        m_sessionPool.Push(&session);
    }
}

BoostNetEngineServer::~BoostNetEngineServer()
{
    Stop();
}

void BoostNetEngineServer::Start(UINT16 port)
{
    m_isRun.store(true);
    Listen(port);
    Update();
}

void BoostNetEngineServer::Listen(UINT16 port)
{
    m_accepter.open(boost::asio::ip::tcp::v4());
    m_accepter.bind(boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port));
    m_accepter.listen();
    Accept();
}

void BoostNetEngineServer::Accept()
{
    if (!m_isRun.load())
        return;

    BoostSession* session = nullptr;
    if (false == m_sessionPool.TryPop(session))
    {
        RetryAccept();
        return;
    }

    m_accepter.async_accept(session->GetSocket(), [this, session](const ErrorCode& errorCode)
    {
        this->CompleteAccept(session, errorCode);
    });
}

void BoostNetEngineServer::RetryAccept()
{
    static constexpr UINT32 kWaitTime = 100;
    m_acceptRetryTimer.expires_after(std::chrono::milliseconds(kWaitTime));
    m_acceptRetryTimer.async_wait([this](const ErrorCode& errorCode)
    {
        if (errorCode)
        {
            // TODO : LOG_ERROR("Accept retry error: {}", errorCode.message());
            return;
        }

        this->Accept();
    });
}

void BoostNetEngineServer::CompleteAccept(BoostSession* session, const ErrorCode& errorCode)
{
    if (!m_isRun.load())
    {
        m_sessionPool.Push(session);
        return;
    }

    if (errorCode)
    {
        m_sessionPool.Push(session);

        if (errorCode != boost::asio::error::operation_aborted)
        {
            // TODO : LOG_ERROR("Accept error: {}", errorCode.message());
            Accept();
        }
        return;
    }

    session->Start();
    Accept();
}

void BoostNetEngineServer::Update()
{
    for (UINT32 i = 0; i < m_threads.size(); ++i)
    {
        m_threads[i].Start("BoostNetEngine_IO_Worker_" + std::to_string(i), [this]()
        {
            m_ioContext.run();
        });
    }
}

void BoostNetEngineServer::Stop()
{
    bool expected = true;                                            // 기대값
    bool desired = false;                                            // 변경값
    if (false == m_isRun.compare_exchange_strong(expected, desired)) // 변경되었다면 true 반환
        return;

    m_accepter.close();
    m_acceptRetryTimer.cancel();
    m_ioContext.stop();
    m_workGuard.reset();

    for (Thread& thread : m_threads)
    {
        thread.Join();
    }
}