#include "Boost/BoostNetEngineServer.h"

BoostNetEngineServer::BoostNetEngineServer(UINT16 port, UINT32 sessionCount, UINT32 threadCount /*=1*/)
    : BoostNetEngine(sessionCount, threadCount)
    , m_port(port)
    , m_accepter(GetIoContext())
    , m_acceptRetryTimer(GetIoContext())
{
}

BoostNetEngineServer::~BoostNetEngineServer()
{
    BoostNetEngine::Stop();
}

void BoostNetEngineServer::OnStart()
{
    using BoostTcp = boost::asio::ip::tcp;
    m_accepter.open(BoostTcp::v4());
    m_accepter.bind(BoostTcp::endpoint(BoostTcp::v4(), m_port));
    m_accepter.listen();
    Accept();
}

void BoostNetEngineServer::Accept()
{
    if (false == IsRunning())
        return;

    BoostSession* session = nullptr;
    if (false == TryPopSession(session))
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
    if (false == IsRunning())
    {
        PushSession(session);
        return;
    }

    if (errorCode)
    {
        PushSession(session);
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

void BoostNetEngineServer::OnStop()
{
    m_accepter.close();
    m_acceptRetryTimer.cancel();
}