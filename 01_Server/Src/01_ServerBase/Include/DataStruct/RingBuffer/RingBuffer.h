#pragma once

#include "DataStruct/RingBuffer/RingBufferWriter.h"
#include "DataStruct/RingBuffer/RingBufferReader.h"

class RingBuffer
{
    friend class RingBufferWriter;
    friend class RingBufferReader;

public:
    RingBuffer() = delete;
    RingBuffer(UINT32 bufferSize);
    ~RingBuffer();

    RingBuffer(const RingBuffer&) = delete;
    RingBuffer(RingBuffer&&) = delete;
    RingBuffer& operator=(const RingBuffer&) = delete;
    RingBuffer& operator=(RingBuffer&&) = delete;

    RingBufferWriter ReserveWrite(UINT32 reserveSize);
    void CommitWrite(RingBufferWriter& writer);
    RingBufferReader ReserveRead(UINT32 readSize);
    void CommitRead(RingBufferReader& reader);

    UINT32 GetWritableSize() const;
    UINT32 GetReadableSize() const;

    UINT32 Read(void* readBuffer, UINT32 readSize);
    UINT32 Peek(void* peekBuffer, UINT32 peekSize) const;

    bool IsEmpty() const
    {
        return m_writeCursor == m_readCursor;
    }

    bool IsFull() const
    {
        return (m_writeCursor + 1) % kCapacity == m_readCursor;
    }

    void Clear();

private:
    using Chunk = std::pair<UINT32, UINT32>;
    Chunk GetWritableChunkSizes() const;
    Chunk GetReadableChunkSizes() const;

    bool m_isActiveWriter;
    bool m_isActiveReader;
    UPtr<char[]> m_buffer;
    UINT32 m_readCursor;
    UINT32 m_writeCursor;
    const UINT32 kCapacity;
};
