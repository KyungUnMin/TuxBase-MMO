#pragma once

class RingBuffer;
class RingBufferWriter
{
    friend class RingBuffer;

    RingBufferWriter();
    RingBufferWriter(RingBuffer* owner, void* ptr, UINT32 size);

public:
    ~RingBufferWriter();

    RingBufferWriter(const RingBufferWriter&) = delete;
    RingBufferWriter& operator=(const RingBufferWriter&) = delete;

    RingBufferWriter(RingBufferWriter&&) noexcept;
    RingBufferWriter& operator=(RingBufferWriter&&) noexcept;

    bool IsValid() const;
    template <typename T = void>
    T* GetPtr(UINT32 offset = 0) const
    {
        ASSERT(offset <= m_size, "Offset out of bounds");
        return reinterpret_cast<T*>(static_cast<BYTE*>(m_ptr) + offset);
    }
    UINT32 GetSize() const { return m_size; }

    template <typename T>
    T& As()
    {
        ASSERT(IsValid(), "Writer is invalid");
        return *static_cast<T*>(m_ptr);
    }

    void Commit(UINT32 writeSize = 0);
    void GiveUp();

private:
    void Clear();

private:
    RingBuffer* m_owner;
    void* m_ptr;
    UINT32 m_size;
};
