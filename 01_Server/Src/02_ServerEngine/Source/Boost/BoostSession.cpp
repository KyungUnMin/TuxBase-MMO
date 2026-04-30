#include "Boost/BoostSession.h"
#include <iostream>

BoostSession::BoostSession(BoostNetEngine& netEngine, IoContext& ioContext)
    : m_socket(ioContext)
    , m_netEngine(netEngine)
{
}

void BoostSession::Start()
{
    m_socket.set_option(boost::asio::ip::tcp::no_delay(true));
    // m_socket.set_option(boost::asio::socket_base::keep_alive(true));  <- 하트비트로 대체
    m_socket.set_option(boost::asio::socket_base::linger(true, 0));
    std::cout << "BoostSessionStart" << std::endl;
}

void BoostSession::CloseSocket()
{
    boost::system::error_code errorCode;
    m_socket.shutdown(Socket::shutdown_both, errorCode);
    m_socket.close(errorCode);
}