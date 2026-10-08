#include "Boost/BoostSession.h"
#include <iostream>

BoostSession::BoostSession(BoostNetEngine& netEngine, IoContext& ioContext)
    : m_socket(boost::asio::make_strand(ioContext))
    , m_netEngine(netEngine)
    , m_recvBuffer(kRecvBufferSize, PacketHeader::kMaxPacketSize)
    , m_isSending(false)
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

bool BoostSession::Send(std::shared_ptr<const SendBuffer> sendBuffer)
{
    ASSERT(nullptr != sendBuffer, "Send buffer is null");

    bool isOverflow = false;
    bool needFlush = false;
    {
        std::lock_guard<std::mutex> lock(m_sendMutex);
        if (kMaxSendQueueSize <= m_sendQueue.size())
        {
            isOverflow = true;
        }
        else
        {
            m_sendQueue.push_back(std::move(sendBuffer));
            needFlush = (false == m_isSending);
            m_isSending = true;
        }
    }

    if (isOverflow)
    {
        boost::asio::post(m_socket.get_executor(), [this]()
        {
            this->CloseSocket();
        });
        return false;
    }

    if (needFlush)
    {
        boost::asio::post(m_socket.get_executor(), [this]()
        {
            this->FlushSendQueue();
        });
    }
    return true;
}

void BoostSession::FlushSendQueue()
{
    {
        std::lock_guard<std::mutex> lock(m_sendMutex);
        if (m_sendQueue.empty())
        {
            m_isSending = false;
            return;
        }
        m_sendingBuffers.swap(m_sendQueue);
    }

    std::vector<boost::asio::const_buffer> buffers;
    buffers.reserve(m_sendingBuffers.size());
    for (const std::shared_ptr<const SendBuffer>& sendBuffer : m_sendingBuffers)
    {
        const std::span<const BYTE> span = sendBuffer->GetSpan();
        buffers.emplace_back(span.data(), span.size());
    }

    boost::asio::async_write(m_socket, buffers, [this](const ErrorCode& errorCode, std::size_t /*sendSize*/)
    {
        this->CompleteSend(errorCode);
    });
}

void BoostSession::CompleteSend(const ErrorCode& errorCode)
{
    m_sendingBuffers.clear();

    if (errorCode)
    {
        // TODO : LOG_ERROR("Send error: {}", errorCode.message());
        {
            std::lock_guard<std::mutex> lock(m_sendMutex);
            m_sendQueue.clear();
            m_isSending = false;
        }
        CloseSocket();
        return;
    }

    FlushSendQueue();
}