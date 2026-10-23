#pragma once

class SendBuffer;

/*-------------------------------------
  Session
-------------------------------------*/

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

class Session : public ISession
{
public:
    Session() = default;
    ~Session() override = default;

    Session(const Session&) = delete;
    Session(Session&&) = delete;
    Session& operator=(const Session&) = delete;
    Session& operator=(Session&&) = delete;
};
