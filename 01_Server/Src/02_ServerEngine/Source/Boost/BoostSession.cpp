#include "Boost/BoostSession.h"
#include "EngineCommon/Packet.h"

BoostSession::BoostSession(BoostNetEngine& netEngine, IoContext& ioContext)
    : Session()
    , m_socket(ioContext)
    , m_netEngine(netEngine)
    , m_isSending(false)
{
}

void BoostSession::Start()
{
    m_socket.set_option(boost::asio::ip::tcp::no_delay(true));
    // m_socket.set_option(boost::asio::socket_base::keep_alive(true));  <- 하트비트로 대체
    m_socket.set_option(boost::asio::socket_base::linger(true, 0));
    std::cout << "BoostSessionStart" << std::endl;

    boost::asio::post(m_socket.get_executor(), [this]()
    {
        this->Receive();
    });
}

void BoostSession::CloseSocket()
{
    boost::system::error_code errorCode;
    m_socket.shutdown(Socket::shutdown_both, errorCode);
    m_socket.close(errorCode);
}

void BoostSession::Receive()
{
    // const std::span<BYTE> writableSpan = m_recvBuffer.GetWritableSpan();
    // m_socket.async_read_some(boost::asio::buffer(writableSpan.data(), writableSpan.size()), [this](const ErrorCode& errorCode, std::size_t recvSize)
    // {
    //     this->CompleteReceive(errorCode, recvSize);
    // });
}

void BoostSession::CompleteReceive(const ErrorCode& errorCode, std::size_t recvSize)
{
    // if (errorCode)
    // {
    //     // TODO : 세션 정리 후 풀 반납 (P0-3)
    //     CloseSocket();
    //     return;
    // }

    // m_recvBuffer.CommitWrite(static_cast<UINT32>(recvSize));
    // if (false == ProcessRecvPackets())
    // {
    //     // TODO : LOG_ERROR("Invalid packet")
    //     CloseSocket();
    //     return;
    // }

    // m_recvBuffer.Compact();
    // Receive();
}

bool BoostSession::ProcessRecvPackets()
{
    // PacketSerializer kumin_temp_pszer; // 임시.
    // while (UPtr<Packet> packet = kumin_temp_pszer.DeserializePacket(m_recvBuffer))
    // {
    //     // packetDispatcher.EnqueuePacket(Packet(kUnassignedSessionId, header, std::move(body)));
    //     // packet을 엔진의 보관용 큐에 넣는다. 나중에 컨텐츠 스레드가 처리할 예정
    // }

    return true;
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