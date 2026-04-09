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

    RingBufferWriter CreateWriter(UINT32 writeSize);
    RingBufferReader CreateReader(UINT32 readSize);
    void GiveUpWriter(RingBufferWriter& writer);
    void GiveUpReader(RingBufferReader& reader);

private:
    void CommitWrite(RingBufferWriter& writer);
    void CommitRead(RingBufferReader& reader);

    bool IsEmpty() const
    {
        return !m_isFull && (m_writeCursor == m_readCursor);
    }

    void Clear();

    bool m_isActiveWriter;
    bool m_isActiveReader;
    bool m_isFull;
    UPtr<char[]> m_buffer;
    UINT32 m_readCursor;
    UINT32 m_writeCursor;
    UINT32 m_tailCursor;
    const UINT32 kCapacity;
};
