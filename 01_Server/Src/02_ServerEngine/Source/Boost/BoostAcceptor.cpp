#include "Boost/BoostAcceptor.h"
#include "Boost/BoostNetEngine.h"
#include "Boost/BoostSession.h"

BoostAcceptor::BoostAcceptor(UINT16 portNum, BoostNetEngineServer& netEngine, IoContext& ioContext)
    : m_portNum(portNum)
    , m_rawAcceptor(ioContext)
    , m_netEngine(netEngine)
    , m_acceptRetryTimer(ioContext)
{
}

void BoostAcceptor::StartListening()
{
    using BoostTcp = boost::asio::ip::tcp;
    m_rawAcceptor.open(BoostTcp::v4());
    m_rawAcceptor.set_option(BoostTcp::acceptor::reuse_address(true));
    m_rawAcceptor.bind(BoostTcp::endpoint(BoostTcp::v4(), m_portNum));
    m_rawAcceptor.listen();
    AcceptSession();
}

void BoostAcceptor::StopListening()
{
    m_rawAcceptor.close();
    m_acceptRetryTimer.cancel();
}

void BoostAcceptor::AcceptSession()
{
    if (false == m_netEngine.IsRunning())
        return;

    SPtr<BoostSession> session = m_netEngine.TakeBoostSession();
    if (nullptr == session)
    {
        RetryAcception();
        return;
    }

    m_rawAcceptor.async_accept(session->GetSocket(), [this, session](const ErrorCode& errorCode)
    {
        this->CompleteAcception(session, errorCode);
    });
}

void BoostAcceptor::CompleteAcception(const SPtr<ISession>& session, const ErrorCode& errorCode)
{
    if (false == m_netEngine.IsRunning())
    {
        // PushSession(session);
        return;
    }

    if (errorCode)
    {
        // PushSession(session);
        if (errorCode != boost::asio::error::operation_aborted)
        {
            // TODO : LOG_ERROR("Accept error: {}", errorCode.message());
            AcceptSession();
        }
        return;
    }

    // session->Start();
    AcceptSession();
}

void BoostAcceptor::RetryAcception()
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

        this->AcceptSession();
    });
}