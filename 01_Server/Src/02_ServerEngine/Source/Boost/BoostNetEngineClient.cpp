#include "Boost/BoostNetEngineClient.h"

BoostNetEngineClient::BoostNetEngineClient(std::string_view ip, UINT16 port)
    : m_endpoint(boost::asio::ip::make_address(ip), port)
    , m_isRun(false)
    , m_workGuard(boost::asio::make_work_guard(m_ioContext))
    , m_socket(m_ioContext)
    , m_retryTimer(m_ioContext)
{
}

BoostNetEngineClient::~BoostNetEngineClient()
{
    Stop();
}

void BoostNetEngineClient::Start()
{
    m_isRun.store(true);
    Connect();

    m_ioThread.Start("BoostNetEngine_Client_IO", [this]()
    {
        m_ioContext.run();
    });
}

void BoostNetEngineClient::Connect()
{
    if (!m_isRun.load())
        return;

    m_socket.async_connect(m_endpoint, [this](const ErrorCode& errorCode)
    {
        this->CompleteConnect(errorCode);
    });
}

void BoostNetEngineClient::CompleteConnect(const ErrorCode& errorCode)
{
    if (!m_isRun.load())
        return;

    if (errorCode)
    {
        if (errorCode != boost::asio::error::operation_aborted)
        {
            // TODO : LOG_ERROR("Connect error: {}", errorCode.message());
            RetryConnect();
        }
        return;
    }

    // TODO : 연결 성공 후 처리 (세션 시작 등)
}

void BoostNetEngineClient::RetryConnect()
{
    static constexpr UINT32 kWaitTime = 1000;
    m_retryTimer.expires_after(std::chrono::milliseconds(kWaitTime));
    m_retryTimer.async_wait([this](const ErrorCode& errorCode)
    {
        if (errorCode)
        {
            // TODO : LOG_ERROR("Retry timer error: {}", errorCode.message());
            return;
        }

        m_socket.close();
        this->Connect();
    });
}

void BoostNetEngineClient::Stop()
{
    bool expected = true;
    bool desired = false;
    if (false == m_isRun.compare_exchange_strong(expected, desired))
        return;

    m_retryTimer.cancel();

    ErrorCode errorCode;
    m_socket.shutdown(Socket::shutdown_both, errorCode);
    m_socket.close(errorCode);

    m_ioContext.stop();
    m_workGuard.reset();
    m_ioThread.Join();
}
