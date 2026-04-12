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

// ============================================================================
// Wrap-around 시나리오 테스트
// ============================================================================

// 커서 리셋 최적화 검증: 모든 데이터를 읽으면(R==W) 커서가 0으로 리셋되어
// 다음 쓰기가 항상 처음부터 시작 → wrap 자체가 발생하지 않음
TEST(RingBufferWrapTest, CursorResetPreventsWrap)
{
    RingBuffer rb(32);

    // 1) 24바이트 쓰기 → W=24
    {
        RingBufferWriter w = rb.CreateWriter(24);
        ASSERT_TRUE(w.IsValid());
        memset(w.GetPtr(), 0xAA, 24);
        w.Commit();
    }

    // 2) 24바이트 전부 읽기 → R==W → Clear()로 양쪽 0으로 리셋
    {
        RingBufferReader r = rb.CreateReader(24);
        ASSERT_TRUE(r.IsValid());
        r.Commit();
    }

    // 3) 리셋 후이므로, 꼬리(32-24=8)가 부족했을 상황이지만
    //    커서가 0으로 돌아왔으므로 24바이트 쓰기가 wrap 없이 성공
    {
        RingBufferWriter w = rb.CreateWriter(24);
        ASSERT_TRUE(w.IsValid());
        memset(w.GetPtr(), 0xBB, 24);
        w.Commit();
    }

    // 4) 데이터 무결성 확인
    {
        RingBufferReader r = rb.CreateReader(24);
        ASSERT_TRUE(r.IsValid());
        const BYTE* data = static_cast<const BYTE*>(r.GetPtr());
        for (UINT32 i = 0; i < 24; ++i)
        {
            EXPECT_EQ(data[i], 0xBB) << "index: " << i;
        }
        r.Commit();
    }
}

// 데이터가 남아있어 리셋이 안 되고 실제 write wrap이 발생하는 경우
// 일부만 읽으면 R≠W이므로 커서 리셋 불가 → 다음 쓰기에서 꼬리 부족 시 wrap
TEST(RingBufferWrapTest, WriteCursorWrapWithPartialRead)
{
    RingBuffer rb(32);

    // 1) 24바이트 쓰기 → W=24
    {
        RingBufferWriter w = rb.CreateWriter(24);
        ASSERT_TRUE(w.IsValid());
        memset(w.GetPtr(), 0x11, 24);
        w.Commit();
    }

    // 2) 16바이트만 읽기 → R=16, W=24 (데이터 8바이트 남음 → 리셋 불가)
    {
        RingBufferReader r = rb.CreateReader(16);
        ASSERT_TRUE(r.IsValid());
        r.Commit();
    }

    // 3) 12바이트 쓰기 → 꼬리 공간=32-24=8 < 12 → wrap → tail=24, W=12
    {
        RingBufferWriter w = rb.CreateWriter(12);
        ASSERT_TRUE(w.IsValid());
        memset(w.GetPtr(), 0x22, 12);
        w.Commit();
    }

    // 4) 뒤쪽 나머지 읽기: R=16 ~ tail=24 → 8바이트 (0x11)
    {
        RingBufferReader r = rb.CreateReader(8);
        ASSERT_TRUE(r.IsValid());
        EXPECT_EQ(r.GetSize(), 8u);
        const BYTE* data = static_cast<const BYTE*>(r.GetPtr());
        for (UINT32 i = 0; i < 8; ++i)
        {
            EXPECT_EQ(data[i], 0x11) << "tail-region index: " << i;
        }
        r.Commit(); // R이 tail(24)에 도달 → 0으로 점프
    }

    // 5) 앞쪽 데이터 읽기: R=0 ~ W=12 → 12바이트 (0x22)
    {
        RingBufferReader r = rb.CreateReader(12);
        ASSERT_TRUE(r.IsValid());
        EXPECT_EQ(r.GetSize(), 12u);
        const BYTE* data = static_cast<const BYTE*>(r.GetPtr());
        for (UINT32 i = 0; i < 12; ++i)
        {
            EXPECT_EQ(data[i], 0x22) << "front-region index: " << i;
        }
        r.Commit();
    }
}

// 여러 사이클 반복: 리셋 최적화 덕분에 매번 wrap 없이 동작하는지 확인
TEST(RingBufferWrapTest, MultipleResetCycles)
{
    constexpr UINT32 kBufSize = 32;
    constexpr UINT32 kChunkSize = 20;
    RingBuffer rb(kBufSize);

    for (UINT32 cycle = 0; cycle < 10; ++cycle)
    {
        const BYTE fillValue = static_cast<BYTE>(cycle + 1);

        {
            RingBufferWriter w = rb.CreateWriter(kChunkSize);
            ASSERT_TRUE(w.IsValid()) << "Write failed at cycle " << cycle;
            memset(w.GetPtr(), fillValue, kChunkSize);
            w.Commit();
        }

        {
            RingBufferReader r = rb.CreateReader(kChunkSize);
            ASSERT_TRUE(r.IsValid()) << "Read failed at cycle " << cycle;
            EXPECT_EQ(r.GetSize(), kChunkSize);
            const BYTE* data = static_cast<const BYTE*>(r.GetPtr());
            for (UINT32 i = 0; i < kChunkSize; ++i)
            {
                EXPECT_EQ(data[i], fillValue) << "cycle=" << cycle << " i=" << i;
            }
            r.Commit(); // 매번 R==W → 리셋 → 다음 사이클도 wrap 불필요
        }
    }
}

// 버퍼를 정확히 꽉 채운 뒤(Full) 부분 읽기→재쓰기 경계 케이스
TEST(RingBufferWrapTest, FullThenPartialDrain)
{
    RingBuffer rb(32);

    // 1) 32바이트 꽉 채우기 → W=0(modulo), Full
    {
        RingBufferWriter w = rb.CreateWriter(32);
        ASSERT_TRUE(w.IsValid());
        memset(w.GetPtr(), 0xCC, 32);
        w.Commit();
    }

    // 2) Full → 쓰기 불가
    {
        RingBufferWriter w = rb.CreateWriter(1);
        EXPECT_FALSE(w.IsValid());
    }

    // 3) 16바이트 읽기 → R=16
    {
        RingBufferReader r = rb.CreateReader(16);
        ASSERT_TRUE(r.IsValid());
        const BYTE* data = static_cast<const BYTE*>(r.GetPtr());
        for (UINT32 i = 0; i < 16; ++i)
        {
            EXPECT_EQ(data[i], 0xCC);
        }
        r.Commit();
    }

    // 4) W=0, R=16 → 앞쪽에 16바이트 쓰기
    {
        RingBufferWriter w = rb.CreateWriter(16);
        ASSERT_TRUE(w.IsValid());
        memset(w.GetPtr(), 0xDD, 16);
        w.Commit(); // W=16, R=16 → Full
    }

    // 5) 나머지 뒤쪽 16바이트(0xCC) 읽기
    {
        RingBufferReader r = rb.CreateReader(16);
        ASSERT_TRUE(r.IsValid());
        const BYTE* data = static_cast<const BYTE*>(r.GetPtr());
        for (UINT32 i = 0; i < 16; ++i)
        {
            EXPECT_EQ(data[i], 0xCC);
        }
        r.Commit();
    }

    // 6) 앞쪽 16바이트(0xDD) 읽기
    {
        RingBufferReader r = rb.CreateReader(16);
        ASSERT_TRUE(r.IsValid());
        const BYTE* data = static_cast<const BYTE*>(r.GetPtr());
        for (UINT32 i = 0; i < 16; ++i)
        {
            EXPECT_EQ(data[i], 0xDD);
        }
        r.Commit();
    }
}

// Wrap 후 남은 gap보다 큰 쓰기는 실패해야 함
TEST(RingBufferWrapTest, WrapGapOverflow)
{
    RingBuffer rb(32);

    // 1) 24바이트 채우기 → W=24
    {
        RingBufferWriter w = rb.CreateWriter(24);
        ASSERT_TRUE(w.IsValid());
        w.Commit();
    }

    // 2) 8바이트만 읽기 → R=8, W=24
    {
        RingBufferReader r = rb.CreateReader(8);
        ASSERT_TRUE(r.IsValid());
        r.Commit();
    }

    // 3) 꼬리=32-24=8, 앞쪽 gap=8. 12바이트 → 양쪽 다 부족 → 실패
    {
        RingBufferWriter w = rb.CreateWriter(12);
        EXPECT_FALSE(w.IsValid());
    }

    // 4) 8바이트(꼬리에 딱 맞음) → 성공
    {
        RingBufferWriter w = rb.CreateWriter(8);
        ASSERT_TRUE(w.IsValid());
        memset(w.GetPtr(), 0xEE, 8);
        w.Commit();
    }

    // 5) 나머지 전부 읽어서 데이터 검증
    {
        RingBufferReader r = rb.CreateReader(16);
        ASSERT_TRUE(r.IsValid());
        r.Commit();
    }
    {
        RingBufferReader r = rb.CreateReader(8);
        ASSERT_TRUE(r.IsValid());
        const BYTE* data = static_cast<const BYTE*>(r.GetPtr());
        for (UINT32 i = 0; i < 8; ++i)
        {
            EXPECT_EQ(data[i], 0xEE);
        }
        r.Commit();
    }
}

// FakeSession을 통한 실사용 패턴: 버퍼 용량(64)보다 많은 패킷을 배치로
// 보내고 받으며 내부적으로 wrap/리셋이 반복되는 상황
TEST(RingBufferWrapTest, MultiPacketSessionWrap)
{
    FakeSession wrapSession;

    constexpr UINT32 kTotalPackets = 20;
    constexpr UINT32 kBatchSize = 5;

    for (UINT32 batch = 0; batch < kTotalPackets / kBatchSize; ++batch)
    {
        for (UINT32 i = 0; i < kBatchSize; ++i)
        {
            const UINT32 seq = batch * kBatchSize + i;
            RingBufferWriter writer = wrapSession.CreateSendBuffer<DummyPacket>();
            ASSERT_TRUE(writer.IsValid()) << "Send failed at packet " << seq;
            DummyPacket& pkt = writer.As<DummyPacket>();
            pkt.m_size = sizeof(DummyPacket);
            pkt.m_id = 1;
            pkt.m_sequence = seq;
            wrapSession.Send(std::move(writer));
        }

        wrapSession.Recv();
        for (UINT32 i = 0; i < kBatchSize; ++i)
        {
            const UINT32 expectedSeq = batch * kBatchSize + i;
            RingBufferReader reader = wrapSession.FetchRecvBuffer<DummyPacket>();
            ASSERT_TRUE(reader.IsValid()) << "Recv failed at packet " << expectedSeq;
            const DummyPacket& pkt = reader.As<DummyPacket>();
            EXPECT_EQ(pkt.m_sequence, expectedSeq);
            reader.Commit();
        }
    }
}