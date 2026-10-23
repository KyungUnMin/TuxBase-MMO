#include "Boost/BoostNetEngine.h"
#include "Boost/BoostSession.h"

/*-------------------------------------
  BoostNetEngine
-------------------------------------*/

BoostNetEngine::BoostNetEngine(UINT32 ioThreadCount, UINT32 sessionCount)
    : NetEngine(ioThreadCount, sessionCount, std::make_unique<BoostSessionFactory>(*this, m_ioContext))
    , m_workGuard(boost::asio::make_work_guard(m_ioContext))
{
    AddStopEvent(std::bind(&BoostNetEngine::StopEngineEvent, this));
}

BoostNetEngine::~BoostNetEngine()
{
}

void BoostNetEngine::RunThreadWorker()
{
    m_ioContext.run();
}

void BoostNetEngine::StopEngineEvent()
{
    m_ioContext.stop();
    m_workGuard.reset();
}

SPtr<BoostSession> BoostNetEngine::TakeBoostSession()
{
    return std::static_pointer_cast<BoostSession>(TakeSession());
}

/*-------------------------------------
  BoostNetEngineServer
-------------------------------------*/

BoostNetEngineServer::BoostNetEngineServer(UINT16 portNum, UINT32 ioThreadCount, UINT32 sessionCount)
    : BoostNetEngine(ioThreadCount, sessionCount)
    , m_acceptor(portNum, *this, GetIoContext())
{
    AddStartEvent(std::bind(&BoostNetEngineServer::StartEngineEvent, this));
    AddStopEvent(std::bind(&BoostNetEngineServer::StopEngineEvent, this));
}

BoostNetEngineServer::~BoostNetEngineServer()
{
}

void BoostNetEngineServer::StartEngineEvent()
{
    m_acceptor.StartListening();
}

void BoostNetEngineServer::StopEngineEvent()
{
    m_acceptor.StopListening();
}

/*-------------------------------------
  BoostNetEngineClient
-------------------------------------*/

BoostNetEngineClient::BoostNetEngineClient(std::string_view host, UINT16 port, UINT32 ioThreadCount, UINT32 sessionCount)
    : BoostNetEngine(ioThreadCount, sessionCount)
    , m_connectRetryTimer(GetIoContext())
{
    boost::system::error_code errorCode;
    boost::asio::ip::address address = boost::asio::ip::make_address(host, errorCode);
    if (!errorCode)
    {
        m_endpoint = Endpoint(address, port);
        return;
    }

    boost::asio::ip::tcp::resolver resolver(GetIoContext());
    boost::asio::ip::tcp::resolver::results_type results = resolver.resolve(std::string(host), std::to_string(port), errorCode);
    ASSERT(!errorCode && !results.empty(), "Invalid IP Address or Hostname. Host: %s", std::string(host).c_str());
    m_endpoint = results.begin()->endpoint();
}

BoostNetEngineClient::~BoostNetEngineClient()
{
    BoostNetEngine::Stop();
}

// void BoostNetEngineClient::OnStart()
// {
//     BoostSession* session = nullptr;
//     while (TryPopSession(session))
//     {
//         ASSERT(session, "session is null");
//         Connect(session);
//     }
// }

void BoostNetEngineClient::Connect(BoostSession* session)
{
    if (false == IsRunning())
        return;

    ASSERT(session, "session is null");
    // session->GetSocket().async_connect(m_endpoint, [this, session](const ErrorCode& errorCode)
    // {
    //     this->CompleteConnect(session, errorCode);
    // });
}

void BoostNetEngineClient::CompleteConnect(BoostSession* session, const ErrorCode& errorCode)
{
    ASSERT(session, "session is null");
    if (false == IsRunning())
    {
        // PushSession(session);
        return;
    }

    if (errorCode)
    {
        if (errorCode == boost::asio::error::operation_aborted)
        {
            // PushSession(session);
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

// void BoostNetEngineClient::OnStop()
// {
//     m_connectRetryTimer.cancel();
// }
