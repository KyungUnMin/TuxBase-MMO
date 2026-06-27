#pragma once
#include <limits>

#pragma pack(push, 1)
struct PacketHeader
{
    UINT16 m_size = 0;
    UINT16 m_id = 0;

    static constexpr UINT32 kHeaderSize = sizeof(PacketHeader);
};
#pragma pack(pop)

using PacketBody = google::protobuf::Message;

class Packet
{
    class ISession;
    class INetEngine;

public:
    Packet();
    ~Packet();

    Packet(const Packet&) = delete;
    Packet& operator=(const Packet&) = delete;

    Packet(Packet&& other) noexcept;
    Packet& operator=(Packet&& other) noexcept;

    UINT64 GetSessionId() const { return m_sessionId; }
    UINT16 GetPacketId() const { return m_header.m_id; }

    template <typename PacketType>
    PacketType* GetBody() const
    {
        ASSERT(IsValid());
        return static_cast<PacketType*>(m_body);
    }

private:
    bool IsValid() const
    {
        // TODO : 나중에 m_sessionId 검사 추가?
        bool result = true;
        result &= (PacketHeader::kHeaderSize < m_header.m_size);
        result &= (m_body != nullptr);
        return result;
    }

private:
    UINT64 m_sessionId;
    PacketHeader m_header;
    PacketBody* m_body;
};