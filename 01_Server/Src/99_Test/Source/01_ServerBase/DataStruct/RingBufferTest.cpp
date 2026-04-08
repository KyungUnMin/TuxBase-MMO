#include <gtest/gtest.h>
#include "DataStruct/RingBuffer/RingBuffer.h"

TEST(RingBufferTest, InitialState)
{
    RingBuffer buffer(64);
    EXPECT_TRUE(buffer.IsEmpty());
    EXPECT_FALSE(buffer.IsFull());
    EXPECT_EQ(buffer.GetWritableSize(), 64);
    EXPECT_EQ(buffer.GetReadableSize(), 0);
}

TEST(RingBufferTest, BasicWriteRead)
{
    RingBuffer buffer(64);

    const char writeData[] = "Hello, World!";
    const UINT32 kDataSize = sizeof(writeData);
    {
        auto writer = buffer.ReserveWrite(kDataSize);
        ASSERT_TRUE(writer.IsValid());
        EXPECT_FALSE(writer.IsWrapped());
        writer.WriteData(writeData, kDataSize);
    }

    EXPECT_FALSE(buffer.IsEmpty());
    EXPECT_EQ(buffer.GetReadableSize(), kDataSize);

    char readData[64] = {};
    UINT32 bytesRead = buffer.Read(readData, kDataSize);
    EXPECT_EQ(bytesRead, kDataSize);
    EXPECT_STREQ(readData, writeData);
    EXPECT_TRUE(buffer.IsEmpty());
}

TEST(RingBufferTest, FullBuffer)
{
    RingBuffer buffer(16);

    {
        auto writer = buffer.ReserveWrite(16);
        ASSERT_TRUE(writer.IsValid());

        char data[16] = {};
        std::memset(data, 0xAB, 16);
        writer.WriteData(data, 16);
    }

    EXPECT_TRUE(buffer.IsFull());
    EXPECT_EQ(buffer.GetWritableSize(), 0);
    EXPECT_EQ(buffer.GetReadableSize(), 16);

    auto failWriter = buffer.ReserveWrite(1);
    EXPECT_FALSE(failWriter.IsValid());
}

TEST(RingBufferTest, EmptyBufferRead)
{
    RingBuffer buffer(16);

    char readData[16] = {};
    UINT32 bytesRead = buffer.Read(readData, 16);
    EXPECT_EQ(bytesRead, 0);
}

TEST(RingBufferTest, WriterRAII)
{
    RingBuffer buffer(32);

    {
        auto writer = buffer.ReserveWrite(10);
        ASSERT_TRUE(writer.IsValid());
        char data[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        writer.WriteData(data, 10);
        // writer가 여기서 소멸 → 자동 Commit
    }

    EXPECT_EQ(buffer.GetReadableSize(), 10);

    char result[10] = {};
    buffer.Read(result, 10);
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_EQ(result[i], i + 1);
    }
}

TEST(RingBufferTest, WriterMoveSemantics)
{
    RingBuffer buffer(32);

    RingBufferWriter movedWriter;
    EXPECT_FALSE(movedWriter.IsValid());

    {
        auto writer = buffer.ReserveWrite(8);
        ASSERT_TRUE(writer.IsValid());
        char data[8] = {10, 20, 30, 40, 50, 60, 70, 80};
        writer.WriteData(data, 8);

        movedWriter = std::move(writer);
        EXPECT_TRUE(movedWriter.IsValid());
        // 원래 writer는 Clear 됨
    }
    // movedWriter가 아직 살아 있으므로 커밋되지 않음
    // (실제로는 커밋됨 — somelWriter 소멸자에서 커밋 안 함, movedWriter가 가지고 있음)

    // movedWriter 소멸 시 커밋
    {
        auto tempWriter = std::move(movedWriter);
    }

    EXPECT_EQ(buffer.GetReadableSize(), 8);

    char result[8] = {};
    buffer.Read(result, 8);
    EXPECT_EQ(result[0], 10);
    EXPECT_EQ(result[7], 80);
}

TEST(RingBufferTest, PeekDoesNotConsume)
{
    RingBuffer buffer(32);

    {
        auto writer = buffer.ReserveWrite(4);
        ASSERT_TRUE(writer.IsValid());
        char data[4] = {0xA, 0xB, 0xC, 0xD};
        writer.WriteData(data, 4);
    }

    char peekData[4] = {};
    UINT32 peeked = buffer.Peek(peekData, 4);
    EXPECT_EQ(peeked, 4);
    EXPECT_EQ(buffer.GetReadableSize(), 4);

    char readData[4] = {};
    UINT32 read = buffer.Read(readData, 4);
    EXPECT_EQ(read, 4);

    EXPECT_EQ(std::memcmp(peekData, readData, 4), 0);
    EXPECT_TRUE(buffer.IsEmpty());
}

TEST(RingBufferTest, MultipleWriteRead)
{
    RingBuffer buffer(32);

    for (int round = 0; round < 10; ++round)
    {
        char writeData = static_cast<char>(round);
        {
            auto writer = buffer.ReserveWrite(1);
            ASSERT_TRUE(writer.IsValid());
            writer.WriteData(&writeData, 1);
        }

        char readData = 0;
        UINT32 bytesRead = buffer.Read(&readData, 1);
        EXPECT_EQ(bytesRead, 1);
        EXPECT_EQ(readData, writeData);
    }

    EXPECT_TRUE(buffer.IsEmpty());
}

TEST(RingBufferTest, WrapAroundWrite)
{
    RingBuffer buffer(16);

    // 12바이트 적고 8바이트 읽어서 커서를 앞으로 밀기
    {
        auto writer = buffer.ReserveWrite(12);
        ASSERT_TRUE(writer.IsValid());
        char data[12] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
        writer.WriteData(data, 12);
    }

    char temp[8] = {};
    buffer.Read(temp, 8);
    // readCursor=8, writeCursor=12

    // 10바이트 wrap write (버퍼 끝 넘어서 시작 부분으로)
    char wrapData[10] = {20, 21, 22, 23, 24, 25, 26, 27, 28, 29};
    {
        auto writer = buffer.ReserveWrite(10);
        ASSERT_TRUE(writer.IsValid());
        EXPECT_TRUE(writer.IsWrapped());
        writer.WriteData(wrapData, 10);
    }

    // 남은 4바이트(첫 번째 쓰기의 9~12) 읽기
    char remaining[4] = {};
    buffer.Read(remaining, 4);
    EXPECT_EQ(remaining[0], 9);
    EXPECT_EQ(remaining[3], 12);

    // wrap된 10바이트 읽기
    char readWrap[10] = {};
    UINT32 bytesRead = buffer.Read(readWrap, 10);
    EXPECT_EQ(bytesRead, 10);
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_EQ(readWrap[i], 20 + i);
    }

    EXPECT_TRUE(buffer.IsEmpty());
}

TEST(RingBufferTest, ScatterGatherWriteData)
{
    RingBuffer buffer(8);

    // 버퍼를 채웠다 비워서 커서를 중간으로 이동
    {
        auto writer = buffer.ReserveWrite(6);
        ASSERT_TRUE(writer.IsValid());
        char data[6] = {0};
        writer.WriteData(data, 6);
    }
    char discard[6] = {};
    buffer.Read(discard, 6);
    // readCursor=6, writeCursor=6

    // 6바이트 write → wrap 발생 (capacity=9, 6부터 시작하면 끝=9까지 3바이트 + 처음 3바이트)
    {
        auto writer = buffer.ReserveWrite(6);
        ASSERT_TRUE(writer.IsValid());
        EXPECT_TRUE(writer.IsWrapped());

        char data[6] = {10, 20, 30, 40, 50, 60};
        UINT32 written = writer.WriteData(data, 6);
        EXPECT_EQ(written, 6);
    }

    char result[6] = {};
    buffer.Read(result, 6);
    EXPECT_EQ(result[0], 10);
    EXPECT_EQ(result[2], 30);
    EXPECT_EQ(result[5], 60);
}

TEST(RingBufferTest, WriterAsTemplate)
{
    RingBuffer buffer(64);

    struct TestPacket
    {
        UINT32 id;
        UINT32 value;
    };

    {
        auto writer = buffer.ReserveWrite(sizeof(TestPacket));
        ASSERT_TRUE(writer.IsValid());
        EXPECT_FALSE(writer.IsWrapped());

        TestPacket* packet = writer.As<TestPacket>();
        packet->id = 42;
        packet->value = 9999;
    }

    TestPacket readPacket = {};
    buffer.Read(&readPacket, sizeof(TestPacket));
    EXPECT_EQ(readPacket.id, 42);
    EXPECT_EQ(readPacket.value, 9999);
}

TEST(RingBufferTest, ReaderZeroCopy)
{
    RingBuffer buffer(64);

    const char writeData[] = "Zero-Copy Read!";
    const UINT32 kDataSize = sizeof(writeData);
    {
        auto writer = buffer.ReserveWrite(kDataSize);
        ASSERT_TRUE(writer.IsValid());
        writer.WriteData(writeData, kDataSize);
    }

    {
        auto reader = buffer.ReserveRead(kDataSize);
        ASSERT_TRUE(reader.IsValid());
        EXPECT_FALSE(reader.IsWrapped());
        EXPECT_EQ(reader.GetTotalSize(), kDataSize);

        // 버퍼 내부 메모리에 직접 접근
        const char* data = static_cast<const char*>(reader.GetFirstPtr());
        EXPECT_STREQ(data, writeData);
    }
    // reader 소멸 → readCursor 자동 이동

    EXPECT_TRUE(buffer.IsEmpty());
}

TEST(RingBufferTest, ReaderRAII)
{
    RingBuffer buffer(32);

    {
        auto writer = buffer.ReserveWrite(5);
        ASSERT_TRUE(writer.IsValid());
        char data[5] = {1, 2, 3, 4, 5};
        writer.WriteData(data, 5);
    }

    EXPECT_EQ(buffer.GetReadableSize(), 5);

    {
        auto reader = buffer.ReserveRead(5);
        ASSERT_TRUE(reader.IsValid());
        // reader가 여기서 소멸 → 자동 CommitRead
    }

    EXPECT_TRUE(buffer.IsEmpty());
}

TEST(RingBufferTest, ReaderWrapAround)
{
    RingBuffer buffer(16);

    // 커서를 중간으로 이동
    {
        auto writer = buffer.ReserveWrite(12);
        writer.WriteData("123456789012", 12);
    }
    char discard[8] = {};
    buffer.Read(discard, 8);
    // readCursor=8, writeCursor=12

    // wrap되는 데이터 쓰기
    {
        auto writer = buffer.ReserveWrite(10);
        ASSERT_TRUE(writer.IsValid());
        char data[10] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
        writer.WriteData(data, 10);
    }

    // 남은 4바이트 읽기
    buffer.Read(discard, 4);

    // wrap된 10바이트를 ReserveRead로 직접 접근
    {
        auto reader = buffer.ReserveRead(10);
        ASSERT_TRUE(reader.IsValid());
        EXPECT_TRUE(reader.IsWrapped());
        EXPECT_EQ(reader.GetTotalSize(), 10);

        // first chunk 검증
        const char* first = static_cast<const char*>(reader.GetFirstPtr());
        for (UINT32 i = 0; i < reader.GetFirstSize(); ++i)
        {
            EXPECT_EQ(first[i], static_cast<char>((i + 1) * 10));
        }

        // second chunk 검증
        const char* second = static_cast<const char*>(reader.GetSecondPtr());
        UINT32 offset = reader.GetFirstSize();
        for (UINT32 i = 0; i < reader.GetSecondSize(); ++i)
        {
            EXPECT_EQ(second[i], static_cast<char>((offset + i + 1) * 10));
        }
    }

    EXPECT_TRUE(buffer.IsEmpty());
}

TEST(RingBufferTest, ReaderAsTemplate)
{
    RingBuffer buffer(64);

    struct TestPacket
    {
        UINT32 id;
        UINT32 value;
    };

    {
        auto writer = buffer.ReserveWrite(sizeof(TestPacket));
        TestPacket* packet = writer.As<TestPacket>();
        packet->id = 100;
        packet->value = 12345;
    }

    {
        auto reader = buffer.ReserveRead(sizeof(TestPacket));
        ASSERT_TRUE(reader.IsValid());
        EXPECT_FALSE(reader.IsWrapped());

        const TestPacket* packet = reader.As<TestPacket>();
        EXPECT_EQ(packet->id, 100);
        EXPECT_EQ(packet->value, 12345);
    }

    EXPECT_TRUE(buffer.IsEmpty());
}
