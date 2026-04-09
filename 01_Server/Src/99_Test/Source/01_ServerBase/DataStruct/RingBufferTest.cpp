#include <gtest/gtest.h>
#include "DataStruct/RingBuffer/RingBuffer.h"
#include "Network/PacketHeader.h"

RingBuffer ringBuffer(64);

TEST(RingBufferTest, Write)
{
    // 패킷큐에 패킷 작성을 모방
    RingBufferWriter writer = ringBuffer.CreateWriter(sizeof(DummyPacket));
    ASSERT_TRUE(writer.IsValid());

    DummyPacket& packet = writer.As<DummyPacket>();
    packet.m_size = sizeof(DummyPacket);
    packet.m_id = 1;
    packet.m_sequence = 1;
    writer.Commit();
}

TEST(RingBufferTest, Read)
{
    // 패킷큐에서 꺼내서 패킷 전송을 모방
    RingBufferReader reader = ringBuffer.CreateReader(sizeof(PacketHeader));
    const PacketHeader& header = reader.As<PacketHeader>();
    const UINT16 size = header.m_size;
    const UINT16 id = header.m_id;
    header.Clear();


    const DummyPacket& readPacket = reader.As<DummyPacket>();
}
