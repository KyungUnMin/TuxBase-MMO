#pragma once
#include "Boost/BoostNetEngine.h"

class BoostNetEngineClient : public BoostNetEngine
{
    using Endpoint = boost::asio::ip::tcp::endpoint;
    using ErrorCode = boost::system::error_code;
    using BoostTimer = boost::asio::steady_timer;

public:
    BoostNetEngineClient() = delete;
    BoostNetEngineClient(std::string_view ip, UINT16 port, UINT32 sessionCount = 1, UINT32 threadCount = 1);
    ~BoostNetEngineClient() override;

    BoostNetEngineClient(const BoostNetEngineClient&) = delete;
    BoostNetEngineClient(BoostNetEngineClient&&) = delete;
    BoostNetEngineClient& operator=(const BoostNetEngineClient&) = delete;
    BoostNetEngineClient& operator=(BoostNetEngineClient&&) = delete;

    void OnStart() override;
    void OnStop() override;

private:
    void Connect(BoostSession* session);
    void CompleteConnect(BoostSession* session, const ErrorCode& errorCode);
    void RetryConnect(BoostSession* session);

private:
    Endpoint m_endpoint;
    BoostTimer m_connectRetryTimer;
};
