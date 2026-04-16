#pragma once

class ISession
{
public:
    ISession() = default;
    virtual ~ISession() = default;

    ISession(const ISession&) = delete;
    ISession(ISession&&) = delete;
    ISession& operator=(const ISession&) = delete;
    ISession& operator=(ISession&&) = delete;
};