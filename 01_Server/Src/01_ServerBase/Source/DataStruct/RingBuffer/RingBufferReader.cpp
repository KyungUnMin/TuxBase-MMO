#include "DataStruct/RingBuffer/RingBuffer.h"
#include "Verify/Assert.h"

RingBufferReader::RingBufferReader()
    : m_owner(nullptr), m_firstPtr(nullptr), m_firstSize(0),
      m_secondPtr(nullptr), m_secondSize(0), m_totalSize(0)
{
}

RingBufferReader::RingBufferReader(RingBuffer* owner, UINT32 totalSize,
                                   const void* firstPtr, UINT32 firstSize,
                                   const void* secondPtr, UINT32 secondSize)
    : m_owner(owner), m_firstPtr(firstPtr), m_firstSize(firstSize),
      m_secondPtr(secondPtr), m_secondSize(secondSize), m_totalSize(totalSize)
{
    ASSERT(IsValid(), "Ring Buffer Reader is invalid");
    ASSERT(m_firstSize + m_secondSize == m_totalSize,
           "Chunk sizes(%u + %u) don't match total(%u)",
           m_firstSize, m_secondSize, m_totalSize);
}

RingBufferReader::~RingBufferReader()
{
    if (IsValid())
    {
        m_owner->CommitRead(*this);
    }
}

RingBufferReader::RingBufferReader(RingBufferReader&& other) noexcept
    : m_owner(other.m_owner), m_firstPtr(other.m_firstPtr),
      m_firstSize(other.m_firstSize), m_secondPtr(other.m_secondPtr),
      m_secondSize(other.m_secondSize), m_totalSize(other.m_totalSize)
{
    other.Clear();
}

RingBufferReader& RingBufferReader::operator=(RingBufferReader&& other) noexcept
{
    if (this != &other)
    {
        if (IsValid())
        {
            m_owner->CommitRead(*this);
        }

        m_owner = other.m_owner;
        m_firstPtr = other.m_firstPtr;
        m_firstSize = other.m_firstSize;
        m_secondPtr = other.m_secondPtr;
        m_secondSize = other.m_secondSize;
        m_totalSize = other.m_totalSize;
        other.Clear();
    }
    return *this;
}

bool RingBufferReader::IsValid() const
{
    return (nullptr != m_owner) && (nullptr != m_firstPtr) && (0 < m_totalSize);
}

void RingBufferReader::Clear()
{
    m_owner = nullptr;
    m_firstPtr = nullptr;
    m_firstSize = 0;
    m_secondPtr = nullptr;
    m_secondSize = 0;
    m_totalSize = 0;
}
