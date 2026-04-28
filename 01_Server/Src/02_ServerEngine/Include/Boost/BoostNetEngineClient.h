#pragma once
#include "Common/INetEngineClient.h"
#include "Threading/Thread.h"

class BoostNetEngineClient : public INetEngineClient
{
    using IoContext = boost::asio::io_context;
    using Socket = boost::asio::ip::tcp::socket;
    using Endpoint = boost::asio::ip::tcp::endpoint;
    using ErrorCode = boost::system::error_code;
    using WorkGuard = boost::asio::executor_work_guard<IoContext::executor_type>;

public:
    BoostNetEngineClient(std::string_view ip, UINT16 port);
    ~BoostNetEngineClient() override;

    BoostNetEngineClient(const BoostNetEngineClient&) = delete;
    BoostNetEngineClient(BoostNetEngineClient&&) = delete;
    BoostNetEngineClient& operator=(const BoostNetEngineClient&) = delete;
    BoostNetEngineClient& operator=(BoostNetEngineClient&&) = delete;

    void Start() override;
    void Stop() override;

private:
    void Connect();
    void CompleteConnect(const ErrorCode& errorCode);
    void RetryConnect();

private:
    Endpoint m_endpoint;
    std::atomic<bool> m_isRun;
    IoContext m_ioContext;
    WorkGuard m_workGuard;
    Socket m_socket;
    boost::asio::steady_timer m_retryTimer;
    Thread m_ioThread;
};
