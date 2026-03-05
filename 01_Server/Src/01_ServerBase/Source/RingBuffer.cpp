#include "RingBuffer.h"
#include "Assert.h"


bool RingBuffer::IsCursorBetween(UINT32 start, UINT32 cursor, UINT32 end)
{
    if (start <= end)
    {
        return (start <= cursor) && (cursor <= end);
    }

    return (start <= cursor) || (cursor <= end);
}

/*--------------------------------------------------------------------------------
RingBufferWriter
--------------------------------------------------------------------------------*/

RingBufferWriter::Writer()
    : _owner(nullptr), _reservePtr(nullptr), _reserveSize(0),
      _reserveCursor(0) {}

RingBufferWriter::Writer(RingBuffer* owner, void* reservePtr,
                         UINT32 reserveSize, UINT32 reserveCursor)
    : _owner(owner), _reservePtr(reservePtr), _reserveSize(reserveSize),
      _reserveCursor(reserveCursor)
{
    ASSERT(IsValid(), "Ring Buffer Writer is invalid");
}

RingBufferWriter::~Writer()
{
    if (!IsClear())
    {
        _owner->CommitWrite(*this);
    }
}

RingBufferWriter::Writer(RingBufferWriter&& other) noexcept
    : _owner(other._owner), _reservePtr(other._reservePtr),
      _reserveSize(other._reserveSize), _reserveCursor(other._reserveCursor)
{
    ASSERT(IsValid(), "Ring Buffer Writer is invalid");
    other.Clear();
}

RingBufferWriter& RingBufferWriter::operator=(Writer&& other) noexcept
{
    _owner = other._owner;
    _reservePtr = other._reservePtr;
    _reserveSize = other._reserveSize;
    _reserveCursor = other._reserveCursor;
    ASSERT(IsValid(), "Ring Buffer Writer is invalid");
    other.Clear();
    return *this;
}

bool RingBufferWriter::IsValid() const
{
    return (nullptr != _owner) && (nullptr != _reservePtr) && (0 < _reserveSize);
}

void RingBufferWriter::Clear()
{
    _owner = nullptr;
    _reservePtr = nullptr;
    _reserveSize = 0;
}

bool RingBufferWriter::IsClear() const
{
    return (nullptr == _owner) && (nullptr == _reservePtr) && (0 == _reserveSize);
}

bool RingBufferWriter::IsWrapped() const
{
    ASSERT(IsValid(), "RingBufferWriter is invalid");
    return _owner->kAllocSize < (_reserveCursor + _reserveSize);
}

/*--------------------------------------------------------------------------------
RingBuffer
--------------------------------------------------------------------------------*/

RingBuffer::RingBuffer(UINT32 allocSize)
    : _full(false), _buffer(new char[allocSize]), _readCursor(0),
      _writeCursor(0), _commitCursor(0), _writerCount(0),
      kAllocSize(allocSize)
{
    ASSERT(0 < kAllocSize, "Ring buffer size is 0");
}

RingBuffer::~RingBuffer()
{
    ASSERT(0 == _writerCount, "All RingBufferWriter is not committed yet");
    ASSERT(IsEmpty(), "Buffer is not empty yet");
    delete[] _buffer;
}

RingBuffer::Chunk RingBuffer::GetWritableChunkSizes() const
{
    Chunk chunk;
    if (_full)
    {
        chunk.first = 0;
        chunk.second = 0;
    }
    else if (_readCursor <= _writeCursor)
    {
        chunk.first = (kAllocSize - _writeCursor);
        chunk.second = _readCursor;
    }
    else
    {
        chunk.first = (_readCursor - _writeCursor);
        chunk.second = 0;
    }

    return chunk;
}

RingBuffer::Chunk RingBuffer::GetReadableChunkSizes() const
{
    Chunk chunk;
    if (IsEmpty())
    {
        chunk.first = 0;
        chunk.second = 0;
    }
    else if (_readCursor < _commitCursor)
    {
        chunk.first = (_commitCursor - _readCursor);
        chunk.second = 0;
    }
    else
    {
        chunk.first = (kAllocSize - _readCursor);
        chunk.second = _commitCursor;
    }
    return chunk;
}

RingBufferWriter RingBuffer::ReserveWrite(UINT32 reserveSize)
{
    ASSERT(0 < reserveSize, "Reserve size is 0");

    if (GetBufferFreeSize() < reserveSize)
    {
        return RingBufferWriter();
    }

    const UINT32 kWritableSize = GetWritableSize();
    if (reserveSize <= kWritableSize)
    {
        // NOLINTNEXTLINE
        void* pReservePtr = (_buffer + _writeCursor);
        const UINT32 kReserveCursor = _writeCursor;
        MoveWriteCursor(reserveSize);
        ++_writerCount;
        return RingBufferWriter(this, pReservePtr, reserveSize, kReserveCursor);
    }

    // NOLINTNEXTLINE
    void* wrapReplaceMemory = malloc(reserveSize);
    if (!wrapReplaceMemory)
    {
        ASSERT(false, "malloc failed");
        return RingBufferWriter();
    }

    const UINT32 kReserveCursor = _writeCursor;
    MoveWriteCursor(reserveSize);
    ++_writerCount;
    return RingBufferWriter(this, wrapReplaceMemory, reserveSize, kReserveCursor);
}

UINT32 RingBuffer::Write(const void* data, UINT32 writeSize)
{
    ASSERT(data, "write data pointer is null");
    ASSERT(0 < writeSize, "write size is 0");
    const char* src = static_cast<const char*>(data);

    const Chunk kWritableChunkSizes = GetWritableChunkSizes();
    const UINT32 kBufferFreeSize =
        kWritableChunkSizes.first + kWritableChunkSizes.second;
    writeSize = std::min(kBufferFreeSize, writeSize);
    if (writeSize == 0)
    {
        return 0;
    }

    const UINT32 kFirstCopy = std::min(kWritableChunkSizes.first, writeSize);
    const UINT32 kSecondCopy = writeSize - kFirstCopy;

    if (0 < kFirstCopy)
    {
        // NOLINTNEXTLINE
        std::memcpy(_buffer + _writeCursor, src, kFirstCopy);
    }

    if (0 < kSecondCopy)
    {
        // NOLINTNEXTLINE
        std::memcpy(_buffer, src + kFirstCopy, kSecondCopy);
    }

    MoveWriteCursor(writeSize);
    return writeSize;
}

void RingBuffer::MoveWriteCursor(UINT32 moveSize)
{
    ASSERT(false == _full, "Buffer is already full");
    ASSERT(0 < moveSize, "Move Size is 0");

    const UINT32 kBufferFreeSize = GetBufferFreeSize();
    ASSERT(moveSize <= kBufferFreeSize,
           "moveSize is bigger than total writable size");

    _writeCursor = (_writeCursor + moveSize) % kAllocSize;
    _full = (_writeCursor == _readCursor);
}

void RingBuffer::CommitWrite(RingBufferWriter& writer)
{
    ASSERT(!writer.IsClear(), "RingBufferWriter is already committed");
    ASSERT(writer.IsValid(), "RingBufferWriter is invalid");
    if (!writer.IsWrapped())
    {
        ASSERT(_commitCursor == writer.GetReserveCursor(), "Wrong order commit");
        MoveCommitCursor(writer.GetReserveSize());
        --_writerCount;
        writer.Clear();
        return;
    }

    void* reservePtr = writer.GetReservePtr();
    const UINT32 kReserveSize = writer.GetReserveSize();

    const UINT32 kOriginWriteCursor = _writeCursor;
    _writeCursor = writer.GetReserveCursor();
    const UINT32 kWrittenSize = Write(reservePtr, kReserveSize);
    ASSERT(kWrittenSize == kReserveSize, "Reserve data write fail");
    _writeCursor = kOriginWriteCursor;
    MoveCommitCursor(kReserveSize);

    // NOLINTNEXTLINE
    free(reservePtr);
    --_writerCount;
    writer.Clear();
}

void RingBuffer::MoveCommitCursor(UINT32 moveSize)
{
    const UINT32 kNextCommitCursorPos = (_commitCursor + moveSize) % kAllocSize;

    if (!IsFull())
    {
        ASSERT(IsCursorBetween(_readCursor, kNextCommitCursorPos, _writeCursor),
               "Commit cursor(%u) moved out of [read(%u), write(%u)] range",
               kNextCommitCursorPos, _readCursor, _writeCursor);
    }

    _commitCursor = kNextCommitCursorPos;
}

UINT32 RingBuffer::Read(void* readBuffer, UINT32 readSize)
{
    readSize = Peek(readBuffer, readSize);
    MoveReadCursor(readSize);
    return readSize;
}

UINT32 RingBuffer::Peek(void* peekBuffer, UINT32 peekSize) const
{
    ASSERT(peekBuffer, "Peek data pointer is null");
    ASSERT(0 < peekSize, "Peek size is 0");

    char* dest = static_cast<char*>(peekBuffer);
    const Chunk kReadableChunkSizes = GetReadableChunkSizes();
    const UINT32 kBufferDataSize =
        kReadableChunkSizes.first + kReadableChunkSizes.second;
    peekSize = std::min(kBufferDataSize, peekSize);
    if (0 == peekSize)
    {
        return 0;
    }

    const UINT32 kFirstCopy = std::min(kReadableChunkSizes.first, peekSize);
    const UINT32 kSecondCopy = peekSize - kFirstCopy;

    if (kFirstCopy)
    {
        // NOLINTNEXTLINE
        std::memcpy(dest, _buffer + _readCursor, kFirstCopy);
    }
    if (kSecondCopy)
    {
        // NOLINTNEXTLINE
        std::memcpy(dest + kFirstCopy, _buffer, kSecondCopy);
    }

    return peekSize;
}

void RingBuffer::MoveReadCursor(UINT32 moveSize)
{
    ASSERT(false == IsEmpty(), "Buffer is already empty");
    ASSERT(0 < moveSize, "move size is 0");

    const UINT32 kBufferDataSize = GetBufferDataSize();
    ASSERT(moveSize <= kBufferDataSize,
           "moveSize is bigger than buffer data size");

    const UINT32 kNextReadCursorPos = (_readCursor + moveSize) % kAllocSize;

    if (!IsFull())
    {
        ASSERT(IsCursorBetween(_readCursor, kNextReadCursorPos, _commitCursor),
               "Read cursor exceeded commit cursor");
    }

    _readCursor = kNextReadCursorPos;
    _full = false;
}

void RingBuffer::Clear()
{
    _writeCursor = 0;
    _readCursor = 0;
    _commitCursor = 0;
    _full = false;

    ASSERT(0 == _writerCount, "All RingBufferWriter is not committed yet");
    _writerCount = 0;
}
