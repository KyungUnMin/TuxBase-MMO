#include "DataStruct/RecvBuffer.h"

RecvBuffer::RecvBuffer(UINT32 capacity, UINT32 minWritableSize)
    : m_buffer(capacity)
    , m_minWritableSize(minWritableSize)
    , m_readPos(0)
    , m_writePos(0)
{
    ASSERT(0 < minWritableSize, "Min writable size is 0");
    ASSERT(minWritableSize <= capacity / 2, "Capacity must be at least twice the min writable size");
}

std::span<BYTE> RecvBuffer::GetWritableSpan()
{
    return std::span<BYTE>(m_buffer.data() + m_writePos, GetWritableSize());
}

std::span<const BYTE> RecvBuffer::GetReadableSpan() const
{
    return std::span<const BYTE>(m_buffer.data() + m_readPos, GetReadableSize());
}

void RecvBuffer::CommitWrite(UINT32 writeSize)
{
    ASSERT(writeSize <= GetWritableSize(), "Write size exceeds writable size");
    m_writePos += writeSize;
}

void RecvBuffer::Consume(UINT32 readSize)
{
    ASSERT(readSize <= GetReadableSize(), "Read size exceeds readable size");
    m_readPos += readSize;
}

void RecvBuffer::Compact()
{
    const UINT32 readableSize = GetReadableSize();
    if (0 == readableSize)
    {
        Clear();
        return;
    }

    if (m_minWritableSize <= GetWritableSize())
    {
        return;
    }

    std::memmove(m_buffer.data(), m_buffer.data() + m_readPos, readableSize);
    m_readPos = 0;
    m_writePos = readableSize;
}

void RecvBuffer::Clear()
{
    m_readPos = 0;
    m_writePos = 0;
}