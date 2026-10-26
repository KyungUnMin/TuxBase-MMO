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

    virtual UINT32 GetSessionID() = 0;
    virtual bool Send(std::shared_ptr<const SendBuffer> sendBuffer) = 0;
};

class Session : public ISession, public std::enable_shared_from_this<Session>
{
public:
    Session() = delete;
    explicit Session(UINT32 sessionID)
        : m_sessionID(sessionID)
    {
    }
    ~Session() override = default;

    Session(const Session&) = delete;
    Session(Session&&) = delete;
    Session& operator=(const Session&) = delete;
    Session& operator=(Session&&) = delete;

    UINT32 GetSessionID() final { return m_sessionID; }

private:
    UINT32 m_sessionID;
};
