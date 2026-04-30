#include "Boost/BoostSession.h"
#include <iostream>

BoostSession::BoostSession(BoostNetEngine& netEngine, IoContext& ioContext)
    : m_socket(ioContext)
    , m_netEngine(netEngine)
{
}

void BoostSession::Start()
{
    std::cout << "BoostSessionStart" << std::endl;
}

void BoostSession::CloseSocket()
{
    boost::system::error_code errorCode;
    m_socket.shutdown(Socket::shutdown_both, errorCode);
    m_socket.close(errorCode);
}