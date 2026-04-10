#include <gtest/gtest.h>
#include "DataStruct/RingBuffer/RingBuffer.h"
#include "Network/PacketHeader.h"

class FakeSocket
{
public:
    FakeSocket()
        : _size(0)
    {
        memset(_buffer, 0, sizeof(_buffer));
    }

    UINT32 Recv(void* buf, UINT32 recvLen)
    {
        if (nullptr == buf)
        {
            return 0;
        }

        recvLen = std::min(recvLen, _size);
        memcpy(buf, _buffer, recvLen);

        memmove(_buffer, _buffer + recvLen, _size - recvLen);
        _size -= recvLen;

        return recvLen;
    }
    UINT32 Send(const void* buf, UINT32 sendLen)
    {
        sendLen = std::min(sendLen, (kCapacity - _size));
        memcpy(_buffer + _size, buf, sendLen);
        _size += sendLen;
        _buffer[_size] = '\0';
        return sendLen;
    }

    UINT32 GetDataSize() const { return _size; }

private:
    static constexpr UINT32 kCapacity = 256;
    BYTE _buffer[kCapacity + 1];
    UINT32 _size;
};

class FakeSession
{
public:
    FakeSession()
        : m_sendBuffer(64)
        , m_recvBuffer(64)
    {
    }
    ~FakeSession() = default;

    FakeSession(const FakeSession&) = delete;
    FakeSession& operator=(const FakeSession&) = delete;
    FakeSession(FakeSession&&) = delete;
    FakeSession& operator=(FakeSession&&) = delete;

    template <typename T>
    RingBufferWriter CreateSendBuffer() { return m_sendBuffer.CreateWriter(sizeof(T)); }
    void Send(RingBufferWriter&& writer)
    {
        const void* ptr = writer.GetPtr();
        const UINT32 size = writer.GetSize();
        writer.Commit();
        m_socket.Send(ptr, size);
        SendComplete(size);
    }

    template <typename T>
    RingBufferReader FetchRecvBuffer() { return m_recvBuffer.CreateReader(sizeof(T)); }
    void Recv()
    {
        while (0 < m_socket.GetDataSize())
        {
            RecvComplete();
        }
    }

private:
    void SendComplete(UINT32 sendSize)
    {
        RingBufferReader reader = m_sendBuffer.CreateReader(sendSize);
        ASSERT_TRUE(reader.IsValid());
        EXPECT_EQ(reader.GetSize(), sendSize);
        reader.Commit();
    }

    void RecvComplete()
    {
        RingBufferWriter writer = m_recvBuffer.CreateAllWriter();
        ASSERT_TRUE(writer.IsValid());
        const UINT32 dataSize = writer.GetSize();
        void* ptr = writer.GetPtr();
        UINT32 recvLen = m_socket.Recv(ptr, dataSize);
        writer.Commit(recvLen);
    }

private:
    FakeSocket m_socket;
    RingBuffer m_sendBuffer;
    RingBuffer m_recvBuffer;
};

FakeSession session;
const UINT32 kPacketSequence = 104628;

// 컨텐츠 -> 링버퍼 -> 소켓 버퍼
TEST(RingBufferTest, Send)
{
    RingBufferWriter writer = session.CreateSendBuffer<DummyPacket>();
    ASSERT_TRUE(writer.IsValid());

    DummyPacket& packet = writer.As<DummyPacket>();
    packet.m_size = sizeof(DummyPacket);
    packet.m_id = 1;
    packet.m_sequence = kPacketSequence;
    session.Send(std::move(writer));
}

// 소켓 버퍼 -> 링버퍼 -> 컨텐츠
TEST(RingBufferTest, Read)
{
    session.Recv();
    RingBufferReader headerReader = session.FetchRecvBuffer<PacketHeader>();
    ASSERT_TRUE(headerReader.IsValid());
    const PacketHeader& packetHeader = headerReader.As<PacketHeader>();
    const UINT16 packetSize = packetHeader.m_size;
    const UINT16 packetID = packetHeader.m_id;
    headerReader.GiveUp();

    RingBufferReader packetReader = session.FetchRecvBuffer<DummyPacket>();
    ASSERT_TRUE(packetReader.IsValid());
    const DummyPacket& packet = packetReader.As<DummyPacket>();
    const UINT32 sequence = packet.m_sequence;
    EXPECT_EQ(sequence, kPacketSequence);
    packetReader.Commit();
}