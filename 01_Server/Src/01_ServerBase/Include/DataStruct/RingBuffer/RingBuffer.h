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
    RingBufferWriter CreateAllWriter();
    RingBufferReader CreateAllReader();

private:
    void CommitWrite(RingBufferWriter& writer, UINT32 writeSize = 0);
    void CommitRead(RingBufferReader& reader, UINT32 readSize = 0);
    void GiveUpWriter(RingBufferWriter& writer);
    void GiveUpReader(RingBufferReader& reader);

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
