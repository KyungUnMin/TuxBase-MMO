#pragma once
#include <utility>

namespace common {

using UINT32 = unsigned int;

class RingBuffer
{
    static bool IsCursorBetween(UINT32 start, UINT32 cursor, UINT32 end);

  public:
    class Writer
    {
        friend class RingBuffer;

        Writer();
        Writer(RingBuffer* owner, void* reservePtr, UINT32 reserveSize, UINT32 reserveCursor);

      public:
        ~Writer();

        Writer(Writer&& other) noexcept;
        Writer& operator=(Writer&& other) noexcept;

        Writer(const Writer& other) = delete;
        Writer& operator=(const Writer&) = delete;

        template <typename T>
        T* As()
        {
            return static_cast<T*>(_reservePtr);
        }

      private:
        void* GetReservePtr() const
        {
            return _reservePtr;
        }
        UINT32 GetReserveSize() const
        {
            return _reserveSize;
        }
        UINT32 GetReserveCursor() const
        {
            return _reserveCursor;
        }
        bool IsWrapped() const;
        bool IsValid() const;
        void Clear();
        bool IsClear() const;

        RingBuffer* _owner;
        void* _reservePtr;
        UINT32 _reserveSize;
        UINT32 _reserveCursor;
    };

    RingBuffer() = delete;
    RingBuffer(UINT32 allocSize);
    ~RingBuffer();

    RingBuffer(const RingBuffer& other) = delete;
    RingBuffer(RingBuffer&& other) = delete;
    void operator=(const RingBuffer& other) = delete;
    void operator=(RingBuffer&& other) = delete;

    UINT32 GetWritableSize() const
    {
        return GetWritableChunkSizes().first;
    }

    Writer ReserveWrite(UINT32 reserveSize);
    void CommitWrite(Writer& writer);
    UINT32 Read(void* readBuffer, UINT32 readSize);
    UINT32 Peek(void* peekBuffer, UINT32 peekSize) const;

    // 이런 느낌으로 한번에 sockect.send 할 수 있도록 만들어보자.
    // 비동기 send도 생각해야함. 그래서 따로 CommitRead 함수 만듬.
    // 얘까지 RAII? 얘는 굳이. 동기Send 일땐 일단 직접 호출시키자
    // struct ReadChunk
    // {
    //     ReadChunk(RingBuffer& owner, void* bufferChunk, const unsigned int kSize);
    //     ~ReadChunk();

    //     void CommitRead();

    //     const void* _bufferChunk;
    //     const unsigned int _kSize;

    //   private:
    //     RingBuffer& _owner;
    //     bool _committed;
    // };

    bool IsEmpty() const
    {
        return (!_full) && (_writeCursor == _readCursor) && (_writeCursor == _commitCursor);
    }
    bool IsFull() const
    {
        return _full;
    }
    void Clear();

  private:
    UINT32 Write(const void* data, UINT32 writeSize);

    using Chunk = std::pair<UINT32, UINT32>;
    Chunk GetWritableChunkSizes() const;
    Chunk GetReadableChunkSizes() const;

    UINT32 GetBufferFreeSize() const
    {
        const Chunk kWritableChunkSizes = GetWritableChunkSizes();
        return kWritableChunkSizes.first + kWritableChunkSizes.second;
    }
    UINT32 GetBufferDataSize() const
    {
        const Chunk kReadableChunkSizes = GetReadableChunkSizes();
        return kReadableChunkSizes.first + kReadableChunkSizes.second;
    }

    void MoveWriteCursor(UINT32 moveSize);
    void MoveReadCursor(UINT32 moveSize);
    void MoveCommitCursor(UINT32 moveSize);

    bool _full;
    char* _buffer;
    UINT32 _readCursor;
    UINT32 _writeCursor;
    UINT32 _commitCursor;
    UINT32 _writerCount;
    const UINT32 kAllocSize;
};

using RingBufferWriter = RingBuffer::Writer;

} // namespace common
