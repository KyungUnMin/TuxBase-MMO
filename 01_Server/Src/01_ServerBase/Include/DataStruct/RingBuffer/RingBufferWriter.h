#pragma once

class RingBuffer;
class RingBufferWriter
{
    friend class RingBuffer;

    RingBufferWriter(RingBuffer* owner, UINT32 totalSize,
                     void* firstPtr, UINT32 firstSize,
                     void* secondPtr, UINT32 secondSize);

public:
    RingBufferWriter();
    ~RingBufferWriter();

    RingBufferWriter(RingBufferWriter&& other) noexcept;
    RingBufferWriter& operator=(RingBufferWriter&& other) noexcept;
    RingBufferWriter(const RingBufferWriter&) = delete;
    RingBufferWriter& operator=(const RingBufferWriter&) = delete;

    bool IsValid() const;

    bool IsWrapped() const
    {
        return m_secondPtr != nullptr;
    }

    template <typename T>
    T* As()
    {
        ASSERT(IsValid(), "Writer is invalid");
        ASSERT(!IsWrapped(), "Cannot use As<T>() on wrapped write");
        return static_cast<T*>(m_firstPtr);
    }

    UINT32 WriteData(const void* data, UINT32 size);

private:
    void Clear();

    RingBuffer* m_owner;
    void* m_firstPtr;
    UINT32 m_firstSize;
    void* m_secondPtr;
    UINT32 m_secondSize;
    UINT32 m_totalSize;
};
