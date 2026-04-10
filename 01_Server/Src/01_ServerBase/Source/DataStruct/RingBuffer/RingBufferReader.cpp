#include "DataStruct/RingBuffer/RingBuffer.h"
#include "Verify/Assert.h"

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
