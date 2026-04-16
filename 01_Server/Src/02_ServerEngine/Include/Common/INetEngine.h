#pragma once

class INetEngine
{
public:
    INetEngine() = default;
    virtual ~INetEngine() = default;

    INetEngine(const INetEngine&) = delete;
    INetEngine(INetEngine&&) = delete;
    INetEngine& operator=(const INetEngine&) = delete;
    INetEngine& operator=(INetEngine&&) = delete;
};