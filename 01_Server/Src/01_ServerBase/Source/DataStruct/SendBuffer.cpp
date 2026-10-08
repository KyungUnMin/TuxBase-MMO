#include "DataStruct/SendBuffer.h"

SendBuffer::SendBuffer(UINT32 size)
    : m_data(size)
{
    ASSERT(0 < size, "Send buffer size is 0");
}