#include "DataStruct/RingBuffer.h"
#include "Verify/Assert.h"

/*-------------------------------------
  RingBufferReader
-------------------------------------*/

RingBufferReader::RingBufferReader()
    : m_owner(nullptr)
    , m_ptr(nullptr)
    , m_size(0)
{
}

RingBufferReader::RingBufferReader(RingBuffer* owner, const void* ptr, UINT32 size)
    : m_owner(owner)
    , m_ptr(ptr)
    , m_size(size)
{
    ASSERT(IsValid(), "Ring Buffer Reader is invalid");
}

RingBufferReader::~RingBufferReader()
{
    if (IsValid())
    {
        m_owner->CommitRead(*this);
    }
}

RingBufferReader::RingBufferReader(RingBufferReader&& other) noexcept
    : m_owner(other.m_owner)
    , m_ptr(other.m_ptr)
    , m_size(other.m_size)
{
    ASSERT(IsValid(), "Reader is invalid");
    other.Clear();
}

RingBufferReader& RingBufferReader::operator=(RingBufferReader&& other) noexcept
{
    ASSERT(other.IsValid(), "Reader is invalid");
    ASSERT(m_owner == other.m_owner, "Cannot move reader from different buffer");
    m_owner = other.m_owner;
    m_ptr = other.m_ptr;
    m_size = other.m_size;
    other.Clear();
    return *this;
}

bool RingBufferReader::IsValid() const
{
    return (nullptr != m_owner) && (nullptr != m_ptr) && (0 < m_size);
}

void RingBufferReader::Commit(UINT32 readSize /*= 0*/)
{
    ASSERT(IsValid(), "Reader is invalid");
    m_owner->CommitRead(*this, readSize);
    Clear();
}

void RingBufferReader::GiveUp()
{
    ASSERT(IsValid(), "Reader is invalid");
    m_owner->GiveUpReader(*this);
    Clear();
}

void RingBufferReader::Clear()
{
    m_owner = nullptr;
    m_ptr = nullptr;
    m_size = 0;
}


/*-------------------------------------
  RingBufferWriter
-------------------------------------*/

RingBufferWriter::RingBufferWriter()
    : m_owner(nullptr)
    , m_ptr(nullptr)
    , m_size(0)
{
}

RingBufferWriter::RingBufferWriter(RingBuffer* owner, void* ptr, UINT32 size)
    : m_owner(owner)
    , m_ptr(ptr)
    , m_size(size)
{
    ASSERT(IsValid(), "Ring Buffer Writer is invalid");
}

RingBufferWriter::~RingBufferWriter()
{
    if (IsValid())
    {
        Commit();
    }
}

RingBufferWriter::RingBufferWriter(RingBufferWriter&& other) noexcept
    : m_owner(other.m_owner)
    , m_ptr(other.m_ptr)
    , m_size(other.m_size)
{
    ASSERT(IsValid(), "Writer is invalid");
    other.Clear();
}

RingBufferWriter& RingBufferWriter::operator=(RingBufferWriter&& other) noexcept
{
    ASSERT(other.IsValid(), "Writer is invalid");
    ASSERT(m_owner == other.m_owner, "Cannot move writer from different buffer");
    m_owner = other.m_owner;
    m_ptr = other.m_ptr;
    m_size = other.m_size;
    other.Clear();
    return *this;
}


bool RingBufferWriter::IsValid() const
{
    return (nullptr != m_owner) && (nullptr != m_ptr) && (0 < m_size);
}

void RingBufferWriter::Commit(UINT32 writeSize /* = 0*/)
{
    ASSERT(IsValid(), "Writer is invalid");
    m_owner->CommitWrite(*this, writeSize);
    Clear();
}

void RingBufferWriter::GiveUp()
{
    ASSERT(IsValid(), "Writer is invalid");
    m_owner->GiveUpWriter(*this);
    Clear();
}

void RingBufferWriter::Clear()
{
    m_owner = nullptr;
    m_ptr = nullptr;
    m_size = 0;
}


/*-------------------------------------
  RingBuffer
-------------------------------------*/

RingBuffer::RingBuffer()
    : m_isActiveWriter(false)
    , m_isActiveReader(false)
    , m_isFull(false)
    , m_readCursor(0)
    , m_writeCursor(0)
    , m_tailCursor(kCapacity)
{
}

RingBuffer::~RingBuffer()
{
    ASSERT(!m_isActiveWriter, "Writer is still active");
    ASSERT(!m_isActiveReader, "Reader is still active");
    ASSERT(IsEmpty(), "Buffer is not empty yet");
}

RingBufferWriter RingBuffer::CreateWriter(UINT32 writeSize)
{
    ASSERT(0 < writeSize, "Write size is 0");
    ASSERT(!m_isActiveWriter, "Writer is already active");

    if (m_isFull)
    {
        return RingBufferWriter();
    }

    // 데이터가 순차적으로 연속 배치된 경우
    if (m_readCursor <= m_writeCursor)
    {
        // 꼬리에 충분한 공간이 있으면 그대로 사용
        if (writeSize <= kCapacity - m_writeCursor) // if (m_writeCursor + writeSize <= kCapacity) 와 같지만 오버플로 방지
        {
            m_isActiveWriter = true;
            return RingBufferWriter(this, m_buffer.data() + m_writeCursor, writeSize);
        }

        // 꼬리 공간 부족. 꼬리 건너뛰고 앞에서 시작
        if (writeSize <= m_readCursor)
        {
            m_tailCursor = m_writeCursor;
            m_writeCursor = 0;
            m_isActiveWriter = true;
            return RingBufferWriter(this, m_buffer.data(), writeSize);
        }
    }

    // 쓰기 커서가 역전하여 앞쪽부터 다시 쓰는 경우
    else
    {
        const UINT32 kRemainSize = m_readCursor - m_writeCursor;
        if (writeSize <= kRemainSize)
        {
            m_isActiveWriter = true;
            return RingBufferWriter(this, m_buffer.data() + m_writeCursor, writeSize);
        }
    }

    return RingBufferWriter();
}

RingBufferReader RingBuffer::CreateReader(UINT32 readSize)
{
    ASSERT(0 < readSize, "Read size is 0");
    ASSERT(!m_isActiveReader, "Reader is already active");

    // tailCursor에 도달했으면 0으로 점프
    if (m_readCursor == m_tailCursor)
    {
        m_readCursor = 0;
        m_tailCursor = kCapacity;
    }

    if (IsEmpty())
    {
        return RingBufferReader();
    }

    // 연속으로 읽을 수 있는 크기 계산
    UINT32 contiguousSize;
    if (m_readCursor < m_writeCursor)
    {
        contiguousSize = m_writeCursor - m_readCursor;
    }
    else
    {
        contiguousSize = m_tailCursor - m_readCursor;
    }

    readSize = std::min(readSize, contiguousSize);
    if (readSize == 0)
    {
        return RingBufferReader();
    }

    m_isActiveReader = true;
    return RingBufferReader(this, m_buffer.data() + m_readCursor, readSize);
}

RingBufferWriter RingBuffer::CreateAllWriter()
{
    ASSERT(!m_isActiveWriter, "Writer is already active");

    if (m_isFull)
    {
        return RingBufferWriter();
    }

    // 연속으로 쓸 수 있는 크기 계산
    UINT32 contiguousSize;
    if (m_readCursor <= m_writeCursor)
    {
        contiguousSize = kCapacity - m_writeCursor;
    }
    else
    {
        contiguousSize = m_readCursor - m_writeCursor;
    }

    if (contiguousSize == 0)
    {
        return RingBufferWriter();
    }

    m_isActiveWriter = true;
    return RingBufferWriter(this, m_buffer.data() + m_writeCursor, contiguousSize);
}

RingBufferReader RingBuffer::CreateAllReader()
{
    ASSERT(!m_isActiveReader, "Reader is already active");

    // tailCursor에 도달했으면 0으로 점프
    if (m_readCursor == m_tailCursor)
    {
        m_readCursor = 0;
        m_tailCursor = kCapacity;
    }

    if (IsEmpty())
    {
        return RingBufferReader();
    }

    // 연속으로 읽을 수 있는 크기 계산
    UINT32 contiguousSize;
    if (m_readCursor < m_writeCursor)
    {
        contiguousSize = m_writeCursor - m_readCursor;
    }
    else
    {
        contiguousSize = m_tailCursor - m_readCursor;
    }

    if (contiguousSize == 0)
    {
        return RingBufferReader();
    }

    m_isActiveReader = true;
    return RingBufferReader(this, m_buffer.data() + m_readCursor, contiguousSize);
}


void RingBuffer::GiveUpWriter(RingBufferWriter& writer)
{
    ASSERT(writer.IsValid(), "Writer is invalid");
    ASSERT(m_isActiveWriter, "No active writer");
    m_isActiveWriter = false;
    writer.Clear();
}
void RingBuffer::GiveUpReader(RingBufferReader& reader)
{
    ASSERT(reader.IsValid(), "Reader is invalid");
    ASSERT(m_isActiveReader, "No active reader");
    m_isActiveReader = false;
    reader.Clear();
}

void RingBuffer::CommitWrite(RingBufferWriter& writer, UINT32 writeSize /* = 0*/)
{
    ASSERT(writer.IsValid(), "Writer is invalid");
    ASSERT(m_isActiveWriter, "No active writer");

    writeSize = (0 == writeSize) ? writer.GetSize() : writeSize;
    ASSERT(writeSize <= writer.GetSize(), "Write size exceeds reserved size");
    ASSERT(m_writeCursor + writeSize <= kCapacity, "Write cursor overflow");

    m_writeCursor = (m_writeCursor + writeSize) % kCapacity;
    m_isFull = (m_writeCursor == m_readCursor);
    m_isActiveWriter = false;
    writer.Clear();
}

void RingBuffer::CommitRead(RingBufferReader& reader, UINT32 readSize /* = 0*/)
{
    ASSERT(reader.IsValid(), "Reader is invalid");
    ASSERT(m_isActiveReader, "No active reader");

    readSize = (0 == readSize) ? reader.GetSize() : readSize;
    ASSERT(readSize <= reader.GetSize(), "Read size exceeds reserved size");
    m_readCursor += readSize;
    ASSERT(m_readCursor <= m_tailCursor, "Read cursor overflow");

    // readCursor가 tailCursor에 도달하면 0으로 점프
    if (m_readCursor == m_tailCursor)
    {
        m_readCursor = 0;
        m_tailCursor = kCapacity;
    }

    m_isFull = false;
    m_isActiveReader = false;

    if (!m_isActiveWriter && m_readCursor == m_writeCursor)
    {
        Clear();
    }

    reader.Clear();
}

void RingBuffer::Clear()
{
    ASSERT(!m_isActiveWriter, "Writer is still active");
    ASSERT(!m_isActiveReader, "Reader is still active");
    m_writeCursor = 0;
    m_readCursor = 0;
    m_tailCursor = kCapacity;
    m_isFull = false;
}
