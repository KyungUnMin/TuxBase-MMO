#include "DataStruct/SerialGenerator.h"
#include <ctime>

SerialGenerator::SerialGenerator(UINT8 serverID, UINT8 threadID)
    : m_lastTime(0)
    , m_kServerID(serverID)
    , m_kThreadID(threadID)
    , m_baseId(0)
{
}

UINT64 SerialGenerator::CreateSerial()
{
    UINT32 now = static_cast<UINT32>(time(nullptr));
    if (m_lastTime != now)
    {
        m_lastTime = now;
        m_baseId = 0;
    }

    return (static_cast<UINT64>(now) << 32) | (static_cast<UINT64>(m_kServerID) << 24) | (static_cast<UINT64>(m_kThreadID) << 16) | static_cast<UINT64>(m_baseId++);
}