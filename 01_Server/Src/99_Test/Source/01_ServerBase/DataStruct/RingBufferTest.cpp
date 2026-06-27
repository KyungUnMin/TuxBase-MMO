#include <gtest/gtest.h>
#include "DataStruct/RingBuffer/RingBuffer.h"

#pragma pack(push, 1)
struct DummyPacketHeader
{
    UINT16 m_size;
    UINT16 m_id;
};

struct DummyPacket : public DummyPacketHeader
{
    UINT32 m_sequence;
};
#pragma pack(pop)

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
    FakeSession() = default;
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

TEST(RingBufferTest, NetIO)
{
    FakeSession session;
    const UINT32 kPacketSequence = 104628;

    // 컨텐츠 -> 링버퍼 -> 소켓 버퍼
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
    {
        session.Recv();
        RingBufferReader headerReader = session.FetchRecvBuffer<DummyPacketHeader>();
        ASSERT_TRUE(headerReader.IsValid());
        const DummyPacketHeader& packetHeader = headerReader.As<DummyPacketHeader>();
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
}

TEST(RingBufferTest, Wrap)
{
    RingBuffer rb;
    const UINT32 kQuarter = RingBuffer::kCapacity / 4; // 4096
    const UINT32 kHalf = RingBuffer::kCapacity / 2;    // 8192
    const UINT32 kThreeQuarter = kQuarter * 3;         // 12288

    // 3/4 쓰기 -> 1/2 읽기
    // [_][_][_][_] -> [1][1][1][_] -> [_][_][1][_]
    // [RW][_][_][_] -> [R][→][→][W] -> [_][_][R][W]
    {
        RingBufferWriter w = rb.CreateWriter(kThreeQuarter);
        ASSERT_TRUE(w.IsValid());
        memset(w.GetPtr(), 0xFF, w.GetSize());
        w.Commit();

        RingBufferReader r = rb.CreateReader(kHalf);
        ASSERT_TRUE(r.IsValid());
        r.Commit();
    }

    // 버퍼에 저장된 연속된 데이터 크기 기록
    UINT32 continueRemainSize = 0;
    {
        RingBufferReader r = rb.CreateAllReader();
        ASSERT_TRUE(r.IsValid());
        continueRemainSize = r.GetSize();
        r.GiveUp();
    }

    // 1/2 쓰기 -> 꼬리(1/4)보다 크므로 랩 발생(앞쪽부터 작성)
    // [_][_][1][_] -> [2][2][1][_]
    // [_][_][R][W] -> [→][→][WR]![_]
    UINT32 tailSkipDataSize = 0;
    {
        RingBufferWriter w = rb.CreateWriter(kHalf);
        ASSERT_TRUE(w.IsValid());
        memset(w.GetPtr(), 0xFF, w.GetSize());
        tailSkipDataSize = w.GetSize();
    }

    // 모든 데이터를 읽으려고 했으나, 연속된 데이터만 읽음(1/4)
    // [2][2][1][_] -> [2][2][_][_]
    // [→][→][WR]![_] -> [R][→][W][_]
    {
        RingBufferReader r = rb.CreateAllReader();
        ASSERT_TRUE(r.IsValid());
        EXPECT_EQ(r.GetSize(), continueRemainSize);
    }

    // 나머지 읽기
    // [2][2][_][_] -> [_][_][_][_]
    // [R][→][W][_] -> [_][_][RW][_]
    {
        RingBufferReader r = rb.CreateAllReader();
        ASSERT_TRUE(r.IsValid());
        EXPECT_EQ(r.GetSize(), tailSkipDataSize);
    }
}

TEST(RingBufferTest, Wrap_ResetCursor)
{
    RingBuffer rb;
    const UINT32 kHalf = RingBuffer::kCapacity / 2;    // 8192
    const UINT32 kQuarter = RingBuffer::kCapacity / 4; // 4096

    // 1/2 쓰기 (0xFF)
    // [_][_][_][_] -> [F][F][_][_]
    // [RW][_][_][_] -> [R][→][W][_]
    {
        RingBufferWriter w = rb.CreateWriter(kHalf);
        ASSERT_TRUE(w.IsValid());
        memset(w.GetPtr(), 0xFF, w.GetSize());
    }

    // 1/2 읽기 (readCursor == writeCursor → 두 커서 모두 0으로 리셋)
    // [F][F][_][_] -> [f][f][_][_] // f = 읽었지만 메모리에 잔존
    // [R][→][W][_] -> [RW][_][_][_]
    {
        RingBufferReader r = rb.CreateReader(kHalf);
        ASSERT_TRUE(r.IsValid());
    }

    // 1/4 쓰기 (0으로 채움, 리셋된 위치 0부터 시작)
    // [f][f][_][_] -> [0][f][_][_]
    // [RW][_][_][_] -> [R][W][_][_]
    {
        RingBufferWriter w = rb.CreateWriter(kQuarter);
        ASSERT_TRUE(w.IsValid());
        memset(w.GetPtr(), 0, w.GetSize());
    }

    // 1/4 만큼 write 커서만 이동 (기존 0xFF 데이터 유지)
    // [0][f][_][_] -> [0][f][_][_]
    // [R][W][_][_] -> [R][→][W][_]
    {
        RingBufferWriter w = rb.CreateWriter(kQuarter);
        ASSERT_TRUE(w.IsValid());
    }

    // 검증: 커서 리셋 후 위치 0부터 작성됐는지 확인 (앞 = 0x00, 뒤 = 0xFF)
    // [0][f][_][_] -> [o][f][_][_] // o,f = 읽었지만 메모리에 잔존
    // [R][→][W][_] -> [RW][_][_][_]
    {
        RingBufferReader r = rb.CreateAllReader();
        ASSERT_TRUE(r.IsValid());
        const UINT32 section1_start = 0;
        const UINT32 section1_end = r.GetSize() / 2;
        const UINT32 section2_start = section1_end;
        const UINT32 section2_end = r.GetSize();

        const BYTE* data = static_cast<const BYTE*>(r.GetPtr());
        for (UINT32 i = section1_start; i < section1_end; ++i)
            EXPECT_EQ(data[i], 0) << "index: " << i;
        for (UINT32 i = section2_start; i < section2_end; ++i)
            EXPECT_EQ(data[i], 0xFF) << "index: " << i;

        r.Commit();
    }
}