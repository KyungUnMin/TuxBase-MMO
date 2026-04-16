#include "Boost/BoostNetEngine.h"

BoostNetEngine::BoostNetEngine()
{
    m_sessions.reserve(kMaxSessionCount);
    m_sessionPool.reserve(kMaxSessionCount);
    for (UINT32 i = 0; i < kMaxSessionCount; ++i)
    {
        BoostSession& session = m_sessions.emplace_back(*this, m_ioContext);
        m_sessionPool.emplace_back(&session);
    }
}

void BoostNetEngine::Listen()
{
    // 일단 ASSERT 처리하고 부족한 경우는 나중에 생각하자
    ASSERT(!m_sessionPool.empty(), "SessionPool is empty");

    BoostSession* session = m_sessionPool.back();
    m_sessionPool.pop_back();
    ASSERT(session, "Session is null");

    m_accepter.async_accept(session->GetSocket(), std::bind(&BoostNetEngine::HandleAccept, this, session, std::placeholders::_1));
}

void BoostNetEngine::HandleAccept(Session* session, const ErrorCode& errorCode)
{
    if (errorCode)
    {
        // LOG_ERROR("Accept error: {}", errorCode.message());
        m_sessionPool.push_back(session);
    }
    else
    {
        // session->Start();
    }

    Listen();
}
