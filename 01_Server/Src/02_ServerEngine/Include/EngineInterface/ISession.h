#pragma once
#include <memory>

class SendBuffer;

class ISession
{
public:
    ISession() = default;
    virtual ~ISession() = default;

    ISession(const ISession&) = delete;
    ISession(ISession&&) = delete;
    ISession& operator=(const ISession&) = delete;
    ISession& operator=(ISession&&) = delete;

    virtual bool Send(std::shared_ptr<const SendBuffer> sendBuffer) = 0;
};