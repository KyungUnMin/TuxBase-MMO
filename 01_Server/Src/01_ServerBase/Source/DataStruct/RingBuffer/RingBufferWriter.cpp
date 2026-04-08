#include "DataStruct/RingBuffer/RingBuffer.h"
#include "Verify/Assert.h"

RingBufferWriter::RingBufferWriter()
    : m_owner(nullptr), m_firstPtr(nullptr), m_firstSize(0), m_secondPtr(nullptr), m_secondSize(0), m_totalSize(0)
{
}

RingBufferWriter::RingBufferWriter(RingBuffer* owner, UINT32 totalSize, void* firstPtr, UINT32 firstSize, void* secondPtr, UINT32 secondSize)
    : m_owner(owner), m_firstPtr(firstPtr), m_firstSize(firstSize), m_secondPtr(secondPtr), m_secondSize(secondSize), m_totalSize(totalSize)
{
    ASSERT(IsValid(), "Ring Buffer Writer is invalid");
    ASSERT(m_firstSize + m_secondSize == m_totalSize, "Chunk sizes(%u + %u) don't match total(%u)", m_firstSize, m_secondSize, m_totalSize);
}

RingBufferWriter::~RingBufferWriter()
{
    if (IsValid())
    {
        m_owner->CommitWrite(*this);
    }
}

RingBufferWriter::RingBufferWriter(RingBufferWriter&& other) noexcept
    : m_owner(other.m_owner), m_firstPtr(other.m_firstPtr), m_firstSize(other.m_firstSize), m_secondPtr(other.m_secondPtr), m_secondSize(other.m_secondSize), m_totalSize(other.m_totalSize)
{
    other.Clear();
}

RingBufferWriter& RingBufferWriter::operator=(RingBufferWriter&& other) noexcept
{
    if (this != &other)
    {
        if (IsValid())
        {
            m_owner->CommitWrite(*this);
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

bool RingBufferWriter::IsValid() const
{
    return (nullptr != m_owner) && (nullptr != m_firstPtr) && (0 < m_totalSize);
}

void RingBufferWriter::Clear()
{
    m_owner = nullptr;
    m_firstPtr = nullptr;
    m_firstSize = 0;
    m_secondPtr = nullptr;
    m_secondSize = 0;
    m_totalSize = 0;
}

UINT32 RingBufferWriter::WriteData(const void* data, UINT32 size)
{
    ASSERT(IsValid(), "Writer is invalid");
    ASSERT(data, "data is null");
    ASSERT(0 < size, "size is 0");

    size = std::min(size, m_totalSize);
    const char* src = static_cast<const char*>(data);

    const UINT32 kFirstCopy = std::min(m_firstSize, size);
    const UINT32 kSecondCopy = size - kFirstCopy;

    if (0 < kFirstCopy)
    {
        std::memcpy(m_firstPtr, src, kFirstCopy);
    }
    if (0 < kSecondCopy)
    {
        ASSERT(m_secondPtr, "Second chunk is null but second copy is needed");
        std::memcpy(m_secondPtr, src + kFirstCopy, kSecondCopy);
    }

    return size;
}
