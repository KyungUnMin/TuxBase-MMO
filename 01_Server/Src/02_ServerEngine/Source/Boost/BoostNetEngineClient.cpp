#include "Boost/BoostNetEngineClient.h"

BoostNetEngineClient::BoostNetEngineClient(std::string_view ip, UINT16 port, UINT32 sessionCount /*= 1*/, UINT32 threadCount /*= 1*/)
    : BoostNetEngine(sessionCount, threadCount)
    , m_endpoint(boost::asio::ip::make_address(ip), port)
    , m_connectRetryTimer(GetIoContext())
{
}

BoostNetEngineClient::~BoostNetEngineClient()
{
    BoostNetEngine::Stop();
}

void BoostNetEngineClient::OnStart()
{
    BoostSession* session = nullptr;
    while (TryPopSession(session))
    {
        ASSERT(session, "session is null");
        Connect(session);
    }
}

void BoostNetEngineClient::Connect(BoostSession* session)
{
    if (false == IsRunning())
        return;

    ASSERT(session, "session is null");
    session->GetSocket().async_connect(m_endpoint, [this, session](const ErrorCode& errorCode)
    {
        this->CompleteConnect(session, errorCode);
    });
}

void BoostNetEngineClient::CompleteConnect(BoostSession* session, const ErrorCode& errorCode)
{
    ASSERT(session, "session is null");
    if (false == IsRunning())
    {
        PushSession(session);
        return;
    }

    if (errorCode)
    {
        if (errorCode == boost::asio::error::operation_aborted)
        {
            PushSession(session);
        }
        else
        {
            // TODO : LOG_ERROR("Connect error: {}", errorCode.message());
            RetryConnect(session);
        }
        return;
    }

    session->Start();
}

void BoostNetEngineClient::RetryConnect(BoostSession* session)
{
    static constexpr UINT32 kWaitTime = 1000;
    m_connectRetryTimer.expires_after(std::chrono::milliseconds(kWaitTime));
    m_connectRetryTimer.async_wait([this, session](const ErrorCode& errorCode)
    {
        if (errorCode)
        {
            // TODO : LOG_ERROR("Retry timer error: {}", errorCode.message());
            return;
        }

        session->CloseSocket();
        this->Connect(session);
    });
}

void BoostNetEngineClient::OnStop()
{
    m_connectRetryTimer.cancel();
}
