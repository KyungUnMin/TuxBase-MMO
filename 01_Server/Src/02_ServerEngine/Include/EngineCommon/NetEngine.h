#pragma once
#include "Packet.h"
#include "Threading/Thread.h"
#include "DataStruct/Delegate.h"
#include "DataStruct/Factory.h"
#include "DataStruct/LockStack.h"

class ISession;

class INetEngine : public std::enable_shared_from_this<INetEngine>
{
public:
    INetEngine() = default;
    virtual ~INetEngine() = default;
    INetEngine(const INetEngine&) = delete;
    INetEngine(INetEngine&&) = delete;
    INetEngine& operator=(const INetEngine&) = delete;
    INetEngine& operator=(INetEngine&&) = delete;

    virtual void Start() = 0;
    virtual void Stop() = 0;
};

class NetEngine : public INetEngine
{
public:
    using SessionFactory = IFactory<ISession>;

    NetEngine() = delete;
    NetEngine(UINT32 ioThreadCount, UINT32 sessionCount, UPtr<SessionFactory>&& sessionFactory);
    ~NetEngine() override = default;

    NetEngine(const NetEngine&) = delete;
    NetEngine(NetEngine&&) = delete;
    NetEngine& operator=(const NetEngine&) = delete;
    NetEngine& operator=(NetEngine&&) = delete;

    bool IsRunning() const { return m_isRun.load(); }
    // PacketDispatcher& GetDispatcher() { return m_packetDispatcher; }

    SPtr<ISession> TakeSession();
    void ReleaseSession(SPtr<ISession>&& session);

protected:
    void Start() final;
    void Stop() final;
    virtual void RunThreadWorker() = 0;

    void AddStartEvent(Delegate<void()>::Callback&& event);
    void AddStopEvent(Delegate<void()>::Callback&& event);

private:
    std::atomic<bool> m_isRun;
    Delegate<void()> m_startDelegator;
    Delegate<void()> m_stopDelegator;

    std::vector<Thread> m_threads;
    LockStack<SPtr<ISession>> m_sessionPool;
    PacketDispatcher m_packetDispatcher;
};