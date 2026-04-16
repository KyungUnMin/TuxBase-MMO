#pragma once

// TODO : TLS영역에 할당될 듯?
class SerialGenerator
{
public:
    SerialGenerator() = delete;
    SerialGenerator(UINT8 serverID, UINT8 threadID);
    ~SerialGenerator() = default;

    SerialGenerator(const SerialGenerator&) = delete;
    SerialGenerator(SerialGenerator&&) = delete;
    SerialGenerator& operator=(const SerialGenerator&) = delete;
    SerialGenerator& operator=(SerialGenerator&&) = delete;

    UINT64 CreateSerial();

private:
    UINT32 m_lastTime;
    const UINT8 m_kServerID;
    const UINT8 m_kThreadID;
    UINT16 m_baseId;
};