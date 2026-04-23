#pragma once
#include "Common/INetEngine.h"
#include "Boost/BoostSession.h"
#include "Threading/Thread.h"
#include "DataStruct/Lock/LockStack.h"
#include <array>
#include <optional>

class BoostNetEngine : public INetEngine
{
    static constexpr UINT32 kMaxSessionCount = 1024;

    using IoContext = boost::asio::io_context;
    using Acceptor = boost::asio::ip::tcp::acceptor;
    using ErrorCode = boost::system::error_code;

public:
    BoostNetEngine();
    ~BoostNetEngine() override;

    BoostNetEngine(const BoostNetEngine&) = delete;
    BoostNetEngine(BoostNetEngine&&) = delete;
    BoostNetEngine& operator=(const BoostNetEngine&) = delete;
    BoostNetEngine& operator=(BoostNetEngine&&) = delete;

    void Start(const UINT16 port);
    void Stop();

private:
    void CreateSessions();
    void Listen(const UINT16 port);
    void ListenWorkerThreadFunc();
    void Update();

private:
    std::atomic<bool> m_isRun;
    IoContext m_ioContext;
    Acceptor m_accepter;
    Thread m_acceptThread;
    std::array<std::optional<BoostSession>, kMaxSessionCount> m_sessions;
    LockStack<BoostSession*> m_sessionPool;
};