#pragma once
#include "Boost/BoostSession.h"
#include "EngineCommon/NetEngine.h"

class PacketDispatcher;

class BoostNetEngine : public NetEngine
{
    using IoContext = boost::asio::io_context;
    using WorkGuard = boost::asio::executor_work_guard<IoContext::executor_type>;
    using BoostSessionFactory = Factory<ISession, BoostSession, BoostNetEngine&, IoContext&>;

public:
    BoostNetEngine() = delete;
    BoostNetEngine(UINT32 ioThreadCount, UINT32 sessionCount);
    virtual ~BoostNetEngine();

    BoostNetEngine(const BoostNetEngine&) = delete;
    BoostNetEngine(BoostNetEngine&&) = delete;
    BoostNetEngine& operator=(const BoostNetEngine&) = delete;
    BoostNetEngine& operator=(BoostNetEngine&&) = delete;

protected:
    IoContext& GetIoContext() { return m_ioContext; }

private:
    void RunThreadWorker() final;

private:
    IoContext m_ioContext;
    WorkGuard m_workGuard;
};
