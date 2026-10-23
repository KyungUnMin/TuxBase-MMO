#pragma once

class RingBuffer;

/*-------------------------------------
  RingBufferReader
-------------------------------------*/

class RingBufferReader
{
    friend class RingBuffer;

    RingBufferReader();
    RingBufferReader(RingBuffer* owner, const void* ptr, UINT32 size);

public:
    ~RingBufferReader();

    RingBufferReader(const RingBufferReader&) = delete;
    RingBufferReader& operator=(const RingBufferReader&) = delete;

    RingBufferReader(RingBufferReader&&) noexcept;
    RingBufferReader& operator=(RingBufferReader&&) noexcept;

    bool IsValid() const;
    template <typename T = void>
    const T* GetPtr(UINT32 offset = 0) const
    {
        ASSERT(offset <= m_size, "Offset out of bounds");
        return reinterpret_cast<const T*>(static_cast<const BYTE*>(m_ptr) + offset);
    }
    UINT32 GetSize() const { return m_size; }

    template <typename T>
    const T& As() const
    {
        ASSERT(IsValid(), "Reader is invalid");
        return *static_cast<const T*>(m_ptr);
    }

    void Commit(UINT32 readSize = 0);
    void GiveUp();

private:
    void Clear();

private:
    RingBuffer* m_owner;
    const void* m_ptr;
    UINT32 m_size;
};


/*-------------------------------------
  RingBufferWriter
-------------------------------------*/

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


/*-------------------------------------
  RingBuffer
-------------------------------------*/
class RingBuffer
{
    friend class RingBufferWriter;
    friend class RingBufferReader;

public:
    static constexpr UINT32 kCapacity = 1024 * 16;

    RingBuffer();
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
    std::array<char, kCapacity> m_buffer;
    UINT32 m_readCursor;
    UINT32 m_writeCursor;
    UINT32 m_tailCursor;
};
