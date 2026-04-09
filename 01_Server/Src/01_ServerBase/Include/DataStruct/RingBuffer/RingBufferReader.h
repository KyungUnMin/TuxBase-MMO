#pragma once

class RingBuffer;
class RingBufferReader
{
    friend class RingBuffer;

    RingBufferReader();
    RingBufferReader(RingBuffer* owner, const void* ptr, UINT32 size);

public:
    ~RingBufferReader();

    RingBufferReader(const RingBufferReader&) = delete;
    RingBufferReader& operator=(const RingBufferReader&) = delete;
    RingBufferReader(RingBufferReader&&) = delete;
    RingBufferReader& operator=(RingBufferReader&&) = delete;

    bool IsValid() const;
    const void* GetPtr() const { return m_ptr; }
    UINT32 GetSize() const { return m_size; }

    template <typename T>
    const T& As() const
    {
        ASSERT(IsValid(), "Reader is invalid");
        return *static_cast<const T*>(m_ptr);
    }

    void Clear();

private:
    RingBuffer* m_owner;
    const void* m_ptr;
    UINT32 m_size;
};
