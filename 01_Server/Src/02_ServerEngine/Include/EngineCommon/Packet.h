#pragma once

#pragma pack(push, 1)
struct PacketHeader
{
    UINT16 m_size = 0;
    UINT16 m_id = 0;
    static constexpr UINT32 kHeaderSize = sizeof(m_size) + sizeof(m_id);
    static constexpr UINT32 kMaxPacketSize = 1024 * 8;

    bool IsValid() const { return (0 < m_id) && (0 < m_size) && (kHeaderSize <= m_size) && (m_size < kMaxPacketSize); }
};
#pragma pack(pop)

using PacketBody = google::protobuf::Message;

class Packet
{
public:
    Packet() = delete;
    Packet(PacketHeader packetHeader, UPtr<PacketBody>&& packetBody)
        : m_header(packetHeader)
        , m_body(std::move(packetBody))
    {
    }
    ~Packet() = default;
    Packet(const Packet&) = delete;
    Packet& operator=(const Packet&) = delete;
    Packet(Packet&& other) noexcept = delete;
    Packet& operator=(Packet&& other) noexcept = delete;

    PacketHeader GetHeader() const { return m_header; }
    const UPtr<PacketBody>& GetBody() const { return m_body; }

private:
    PacketHeader m_header;
    UPtr<PacketBody> m_body;
};