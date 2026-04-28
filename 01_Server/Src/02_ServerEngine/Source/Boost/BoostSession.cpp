#include "Boost/BoostSession.h"

BoostSession::BoostSession(BoostNetEngineServer& netEngine, IoContext& ioContext)
    : m_socket(ioContext)
    , m_netEngine(netEngine)
    , m_recvBuffer(kBufferSize)
    , m_sendBuffer(kBufferSize)
{
}

void BoostSession::Start()
{
}