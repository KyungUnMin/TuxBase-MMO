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

/*-------------------------------------
  BoostNetEngineClient
-------------------------------------*/

BoostNetEngineServer::BoostNetEngineServer(UINT16 port, UINT32 ioThreadCount, UINT32 sessionCount)
    : BoostNetEngine(ioThreadCount, sessionCount)
    , m_port(port)
    , m_accepter(GetIoContext())
    , m_acceptRetryTimer(GetIoContext())
{
}

BoostNetEngineServer::~BoostNetEngineServer()
{
    BoostNetEngine::Stop();
}

// void BoostNetEngineServer::OnStart()
// {
//     using BoostTcp = boost::asio::ip::tcp;
//     m_accepter.open(BoostTcp::v4());
//     m_accepter.set_option(BoostTcp::acceptor::reuse_address(true));
//     m_accepter.bind(BoostTcp::endpoint(BoostTcp::v4(), m_port));
//     m_accepter.listen();
//     Accept();
// }

void BoostNetEngineServer::Accept()
{
    if (false == IsRunning())
        return;

    BoostSession* session = nullptr;
    // if (false == TryPopSession(session))
    // {
    //     RetryAccept();
    //     return;
    // }

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
    // if (false == IsRunning())
    // {
    //     PushSession(session);
    //     return;
    // }

    // if (errorCode)
    // {
    //     PushSession(session);
    //     if (errorCode != boost::asio::error::operation_aborted)
    //     {
    //         // TODO : LOG_ERROR("Accept error: {}", errorCode.message());
    //         Accept();
    //     }
    //     return;
    // }

    session->Start();
    Accept();
}

// void BoostNetEngineServer::OnStop()
// {
//     m_accepter.close();
//     m_acceptRetryTimer.cancel();
// }

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
