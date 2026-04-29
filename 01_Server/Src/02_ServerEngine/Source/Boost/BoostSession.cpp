#include "Boost/BoostSession.h"

BoostSession::BoostSession(BoostNetEngine& netEngine, IoContext& ioContext)
    : m_socket(ioContext)
    , m_netEngine(netEngine)
    , m_recvBuffer(kBufferSize)
    , m_sendBuffer(kBufferSize)
{
}

void BoostSession::Start()
{
}

void BoostSession::CloseSocket()
{
    boost::system::error_code errorCode;
    m_socket.shutdown(Socket::shutdown_both, errorCode);
    m_socket.close(errorCode);
}