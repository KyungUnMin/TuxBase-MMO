#include "DataStruct/RingBuffer/RingBuffer.h"
#include "Verify/Assert.h"

/*--------------------------------------------------------------------------------
RingBuffer
--------------------------------------------------------------------------------*/

RingBuffer::RingBuffer(UINT32 bufferSize)
    : m_isActiveWriter(false), m_isActiveReader(false), m_buffer(std::make_unique<char[]>(bufferSize + 1)), m_readCursor(0), m_writeCursor(0), kCapacity(bufferSize + 1)
{
    ASSERT(0 < bufferSize, "Ring buffer size is 0");
}

RingBuffer::~RingBuffer()
{
    ASSERT(!m_isActiveWriter, "Writer is still active");
    ASSERT(!m_isActiveReader, "Reader is still active");
    ASSERT(IsEmpty(), "Buffer is not empty yet");
}

RingBufferWriter RingBuffer::ReserveWrite(UINT32 writeSize)
{
    ASSERT(0 < writeSize, "Reserve size is 0");
    ASSERT(false == m_isActiveWriter, "Writer is already active");

    if (GetWritableSize() < writeSize)
    {
        return RingBufferWriter();
    }

    const Chunk kWritable = GetWritableChunkSizes();
    if (writeSize <= kWritable.first)
    {
        void* ptr = m_buffer.get() + m_writeCursor;
        m_isActiveWriter = true;
        return RingBufferWriter(this, writeSize, ptr, writeSize, nullptr, 0);
    }

    void* firstPtr = m_buffer.get() + m_writeCursor;
    const UINT32 kFirstSize = kWritable.first;
    void* secondPtr = m_buffer.get();
    const UINT32 kSecondSize = writeSize - kFirstSize;

    m_isActiveWriter = true;
    return RingBufferWriter(this, writeSize, firstPtr, kFirstSize, secondPtr, kSecondSize);
}

RingBufferReader RingBuffer::ReserveRead(UINT32 readSize)
{
    ASSERT(0 < readSize, "Read size is 0");
    ASSERT(!m_isActiveReader, "Reader is already active");

    const UINT32 kDataSize = GetReadableSize();
    readSize = std::min(kDataSize, readSize);
    if (0 == readSize)
    {
        return RingBufferReader();
    }

    const Chunk kReadable = GetReadableChunkSizes();

    if (readSize <= kReadable.first)
    {
        const void* ptr = m_buffer.get() + m_readCursor;
        m_isActiveReader = true;
        return RingBufferReader(this, readSize, ptr, readSize, nullptr, 0);
    }

    const void* firstPtr = m_buffer.get() + m_readCursor;
    const UINT32 kFirstSize = kReadable.first;
    const void* secondPtr = m_buffer.get();
    const UINT32 kSecondSize = readSize - kFirstSize;

    m_isActiveReader = true;
    return RingBufferReader(this, readSize, firstPtr, kFirstSize, secondPtr, kSecondSize);
}

RingBuffer::Chunk RingBuffer::GetWritableChunkSizes() const
{
    if (m_readCursor <= m_writeCursor)
    {
        if (m_readCursor == 0)
        {
            return {kCapacity - m_writeCursor - 1, 0};
        }
        return {kCapacity - m_writeCursor, m_readCursor - 1};
    }
    return {m_readCursor - m_writeCursor - 1, 0};
}

RingBuffer::Chunk RingBuffer::GetReadableChunkSizes() const
{
    if (m_readCursor == m_writeCursor)
    {
        return {0, 0};
    }

    if (m_readCursor < m_writeCursor)
    {
        return {m_writeCursor - m_readCursor, 0};
    }
    return {kCapacity - m_readCursor, m_writeCursor};
}

UINT32 RingBuffer::GetWritableSize() const
{
    const Chunk kChunks = GetWritableChunkSizes();
    return kChunks.first + kChunks.second;
}

UINT32 RingBuffer::GetReadableSize() const
{
    const Chunk kChunks = GetReadableChunkSizes();
    return kChunks.first + kChunks.second;
}



void RingBuffer::CommitWrite(RingBufferWriter& writer)
{
    ASSERT(writer.IsValid(), "Writer is invalid");
    ASSERT(m_isActiveWriter, "No active writer");

    m_writeCursor = (m_writeCursor + writer.m_totalSize) % kCapacity;
    m_isActiveWriter = false;
    writer.Clear();
}

void RingBuffer::CommitRead(RingBufferReader& reader)
{
    ASSERT(reader.IsValid(), "Reader is invalid");
    ASSERT(m_isActiveReader, "No active reader");

    m_readCursor = (m_readCursor + reader.m_totalSize) % kCapacity;
    m_isActiveReader = false;
    reader.Clear();
}

UINT32 RingBuffer::Read(void* readBuffer, UINT32 readSize)
{
    readSize = Peek(readBuffer, readSize);
    if (0 < readSize)
    {
        m_readCursor = (m_readCursor + readSize) % kCapacity;
    }
    return readSize;
}

UINT32 RingBuffer::Peek(void* peekBuffer, UINT32 peekSize) const
{
    ASSERT(peekBuffer, "Peek buffer is null");
    ASSERT(0 < peekSize, "Peek size is 0");

    char* dest = static_cast<char*>(peekBuffer);
    const Chunk kReadable = GetReadableChunkSizes();
    const UINT32 kDataSize = kReadable.first + kReadable.second;
    peekSize = std::min(kDataSize, peekSize);
    if (0 == peekSize)
    {
        return 0;
    }

    const UINT32 kFirstCopy = std::min(kReadable.first, peekSize);
    const UINT32 kSecondCopy = peekSize - kFirstCopy;

    if (0 < kFirstCopy)
    {
        // NOLINTNEXTLINE
        std::memcpy(dest, m_buffer.get() + m_readCursor, kFirstCopy);
    }
    if (0 < kSecondCopy)
    {
        // NOLINTNEXTLINE
        std::memcpy(dest + kFirstCopy, m_buffer.get(), kSecondCopy);
    }

    return peekSize;
}

void RingBuffer::Clear()
{
    ASSERT(!m_isActiveWriter, "Writer is still active");
    ASSERT(!m_isActiveReader, "Reader is still active");
    m_writeCursor = 0;
    m_readCursor = 0;
}
