#pragma once
#include "Packet.h"
#include "Threading/Thread.h"
#include "DataStruct/Delegate.h"

class INetEngine
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
    explicit NetEngine(UINT32 threadCount);
    ~NetEngine() override = default;
    NetEngine(const NetEngine&) = delete;
    NetEngine(NetEngine&&) = delete;
    NetEngine& operator=(const NetEngine&) = delete;
    NetEngine& operator=(NetEngine&&) = delete;

    bool IsRunning() const { return m_isRun.load(); }
    PacketDispatcher& GetDispatcher() { return m_packetDispatcher; }

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
    PacketDispatcher m_packetDispatcher;
};