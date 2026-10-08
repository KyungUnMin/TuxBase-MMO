#pragma once
#include <span>

class RecvBuffer
{
public:
    RecvBuffer() = delete;
    RecvBuffer(UINT32 capacity, UINT32 minWritableSize);
    ~RecvBuffer() = default;

    RecvBuffer(const RecvBuffer&) = delete;
    RecvBuffer(RecvBuffer&&) = delete;
    RecvBuffer& operator=(const RecvBuffer&) = delete;
    RecvBuffer& operator=(RecvBuffer&&) = delete;

    std::span<BYTE> GetWritableSpan();
    std::span<const BYTE> GetReadableSpan() const;
    UINT32 GetWritableSize() const { return static_cast<UINT32>(m_buffer.size()) - m_writePos; }
    UINT32 GetReadableSize() const { return m_writePos - m_readPos; }

    void CommitWrite(UINT32 writeSize);
    void Consume(UINT32 readSize);
    void Compact();
    void Clear();

private:
    std::vector<BYTE> m_buffer;
    const UINT32 m_minWritableSize;
    UINT32 m_readPos;
    UINT32 m_writePos;
};