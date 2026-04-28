#pragma once

class INetEngineClient
{
public:
    INetEngineClient() = default;
    virtual ~INetEngineClient() = default;

    INetEngineClient(const INetEngineClient&) = delete;
    INetEngineClient(INetEngineClient&&) = delete;
    INetEngineClient& operator=(const INetEngineClient&) = delete;
    INetEngineClient& operator=(INetEngineClient&&) = delete;

    virtual void Start() = 0;
    virtual void Stop() = 0;
};
