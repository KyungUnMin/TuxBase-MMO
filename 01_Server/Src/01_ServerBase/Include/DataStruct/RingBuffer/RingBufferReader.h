#pragma once

class RingBuffer;

/**
 * @brief 링버퍼의 읽기 예약을 나타내는 RAII 핸들.
 * 소멸 시 자동으로 CommitRead를 호출하여 readCursor를 이동합니다.
 * 버퍼 내부 메모리에 대한 const 포인터를 직접 제공합니다 (Zero-Copy).
 */
class RingBufferReader
{
    friend class RingBuffer;

    RingBufferReader(RingBuffer* owner, UINT32 totalSize,
                     const void* firstPtr, UINT32 firstSize,
                     const void* secondPtr, UINT32 secondSize);

public:
    RingBufferReader();
    ~RingBufferReader();

    RingBufferReader(RingBufferReader&& other) noexcept;
    RingBufferReader& operator=(RingBufferReader&& other) noexcept;

    RingBufferReader(const RingBufferReader&) = delete;
    RingBufferReader& operator=(const RingBufferReader&) = delete;

    bool IsValid() const;

    bool IsWrapped() const
    {
        return m_secondPtr != nullptr;
    }

    const void* GetFirstPtr() const { return m_firstPtr; }
    UINT32 GetFirstSize() const { return m_firstSize; }
    const void* GetSecondPtr() const { return m_secondPtr; }
    UINT32 GetSecondSize() const { return m_secondSize; }
    UINT32 GetTotalSize() const { return m_totalSize; }

    template <typename T>
    const T* As() const
    {
        ASSERT(IsValid(), "Reader is invalid");
        ASSERT(!IsWrapped(), "Cannot use As<T>() on wrapped read");
        return static_cast<const T*>(m_firstPtr);
    }

private:
    void Clear();

    RingBuffer* m_owner;
    const void* m_firstPtr;
    UINT32 m_firstSize;
    const void* m_secondPtr;
    UINT32 m_secondSize;
    UINT32 m_totalSize;
};
