#pragma once
#include "Boost/BoostSession.h"
#include "DataStruct/LockStack.h"
#include "EngineCommon/NetEngine.h"

class PacketDispatcher;

class BoostNetEngine : public NetEngine
{
    using IoContext = boost::asio::io_context;
    using WorkGuard = boost::asio::executor_work_guard<IoContext::executor_type>;

public:
    BoostNetEngine() = delete;
    BoostNetEngine(UINT32 sessionCount, UINT32 threadCount);
    virtual ~BoostNetEngine();

    BoostNetEngine(const BoostNetEngine&) = delete;
    BoostNetEngine(BoostNetEngine&&) = delete;
    BoostNetEngine& operator=(const BoostNetEngine&) = delete;
    BoostNetEngine& operator=(BoostNetEngine&&) = delete;

protected:
    IoContext& GetIoContext() { return m_ioContext; }
    void PushSession(BoostSession* session);
    bool TryPopSession(BoostSession*& session);

private:
    void RunThreadWorker() final;

private:
    IoContext m_ioContext;
    WorkGuard m_workGuard;
    std::vector<std::optional<BoostSession>> m_sessions;
    LockStack<BoostSession*> m_sessionPool;
};
