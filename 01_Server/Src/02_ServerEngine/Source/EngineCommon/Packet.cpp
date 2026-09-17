#include "EngineCommon/Packet.h"

Packet::Packet()
    : m_sessionId(0)
    , m_body(nullptr)
{
}

Packet::Packet(UINT64 sessionId, const PacketHeader& header, std::unique_ptr<PacketBody> body)
    : m_sessionId(sessionId)
    , m_header(header)
    , m_body(std::move(body))
{
}

Packet::~Packet()
{
}

Packet::Packet(Packet&& other) noexcept
    : m_sessionId(std::exchange(other.m_sessionId, 0))
    , m_header(std::exchange(other.m_header, PacketHeader{}))
    , m_body(std::move(other.m_body))
{
}

Packet& Packet::operator=(Packet&& other) noexcept
{
    if (this != &other)
    {
        m_sessionId = std::exchange(other.m_sessionId, 0);
        m_header = std::exchange(other.m_header, PacketHeader{});
        m_body = std::move(other.m_body);
    }
    return *this;
}