#include "DataStruct/RingBuffer/RingBuffer.h"
#include "Verify/Assert.h"

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
