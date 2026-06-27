#pragma once

class ISession;

class INetEngine
{
public:
    INetEngine() = default;
    virtual ~INetEngine() = default;

    INetEngine(const INetEngine&) = delete;
    INetEngine(INetEngine&&) = delete;
    INetEngine& operator=(const INetEngine&) = delete;
    INetEngine& operator=(INetEngine&&) = delete;

    virtual ISession* FindSession(UINT64 serialId) = 0;
};