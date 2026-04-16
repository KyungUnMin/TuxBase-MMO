#include "Boost/BoostSession.h"

BoostSession::BoostSession(BoostNetEngine& netEngine, IoContext& ioContext)
    : m_netEngine(netEngine)
    , m_socket(ioContext)
    , m_recvBuffer(kBufferSize)
    , m_sendBuffer(kBufferSize)
{
}