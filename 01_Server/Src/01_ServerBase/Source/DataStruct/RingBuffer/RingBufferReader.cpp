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

bool RingBufferReader::IsValid() const
{
    return (nullptr != m_owner) && (nullptr != m_ptr) && (0 < m_size);
}

void RingBufferReader::Clear()
{
    m_owner = nullptr;
    m_ptr = nullptr;
    m_size = 0;
}
