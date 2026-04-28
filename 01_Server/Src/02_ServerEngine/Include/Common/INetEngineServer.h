#pragma once

class INetEngineServer
{
public:
    INetEngineServer() = default;
    virtual ~INetEngineServer() = default;

    INetEngineServer(const INetEngineServer&) = delete;
    INetEngineServer(INetEngineServer&&) = delete;
    INetEngineServer& operator=(const INetEngineServer&) = delete;
    INetEngineServer& operator=(INetEngineServer&&) = delete;

    virtual void Start(UINT16 port) = 0;
    virtual void Stop() = 0;
};