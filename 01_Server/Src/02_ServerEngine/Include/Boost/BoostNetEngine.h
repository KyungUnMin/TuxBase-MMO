#pragma once
#include "Common/INetEngine.h"
#include "Boost/BoostSession.h"

class BoostNetEngine : public INetEngine
{
    static constexpr UINT32 kMaxSessionCount = 1024;

    using IoContext = boost::asio::io_context;
    using Acceptor = boost::asio::ip::tcp::acceptor;
    using ErrorCode = boost::system::error_code;

public:
    BoostNetEngine();
    ~BoostNetEngine() override = default;

    BoostNetEngine(const BoostNetEngine&) = delete;
    BoostNetEngine(BoostNetEngine&&) = delete;
    BoostNetEngine& operator=(const BoostNetEngine&) = delete;
    BoostNetEngine& operator=(BoostNetEngine&&) = delete;

    void Listen();

private:
    void HandleAccept(Session* session, const ErrorCode& errorCode);

private:
    IoContext m_ioContext;
    Acceptor m_accepter;

    template <typename T>
    using Stack = std::vector<T>;
    Stack<BoostSession*> m_sessionPool;
    std::vector<BoostSession> m_sessions;
};