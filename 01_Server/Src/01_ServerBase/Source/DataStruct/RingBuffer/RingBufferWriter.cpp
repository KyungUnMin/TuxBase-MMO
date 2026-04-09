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

bool RingBufferWriter::IsValid() const
{
    return (nullptr != m_owner) && (nullptr != m_ptr) && (0 < m_size);
}

void RingBufferWriter::Commit()
{
    ASSERT(IsValid(), "Writer is invalid");
    m_owner->CommitWrite(*this);
    Clear();
}

void RingBufferWriter::Clear()
{
    m_owner = nullptr;
    m_ptr = nullptr;
    m_size = 0;
}
