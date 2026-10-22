#pragma once

class ISession;
class RecvBuffer;

/*-------------------------------------
  Packet
-------------------------------------*/

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

/*-------------------------------------
  PacketParser
-------------------------------------*/

class IPacketParser
{
public:
    IPacketParser() = default;
    virtual ~IPacketParser() = default;
    IPacketParser(const IPacketParser&) = delete;
    IPacketParser(IPacketParser&&) = delete;
    IPacketParser& operator=(const IPacketParser&) = delete;
    IPacketParser& operator=(IPacketParser&&) = delete;

    static PacketHeader ParsePacketHeader(const RecvBuffer& recvBuffer);
    virtual UPtr<PacketBody> ParsePacketBody(std::span<const BYTE> rawPacketBodyData) const = 0;
};

template <typename TPacketType>
class PacketParser : public IPacketParser
{
    static_assert(std::is_base_of<PacketBody, TPacketType>::value, "TPacketType must inherit from PacketBody");

public:
    PacketParser() = default;
    ~PacketParser() = default;
    PacketParser(const PacketParser&) = delete;
    PacketParser(PacketParser&&) = delete;
    PacketParser& operator=(const PacketParser&) = delete;
    PacketParser& operator=(PacketParser&&) = delete;

    UPtr<PacketBody> ParsePacketBody(std::span<const BYTE> rawPacketBodyData) const override;
};

template <typename TPacketType>
UPtr<PacketBody> PacketParser<TPacketType>::ParsePacketBody(std::span<const BYTE> rawPacketBodyData) const
{
    UPtr<TPacketType> packetBody = std::make_unique<TPacketType>();
    if (nullptr == packetBody)
        return nullptr;

    if (false == packetBody->ParseFromArray(rawPacketBodyData.data(), rawPacketBodyData.size_bytes()))
        return nullptr;

    return std::move(packetBody);
}

/*-------------------------------------
            PacketHandler
-------------------------------------*/
class IPacketHandler
{
public:
    IPacketHandler() = default;
    virtual ~IPacketHandler() = default;
    IPacketHandler(const IPacketHandler&) = delete;
    IPacketHandler(IPacketHandler&&) = delete;
    IPacketHandler& operator=(const IPacketHandler&) = delete;
    IPacketHandler& operator=(IPacketHandler&&) = delete;

    virtual bool ProcessPacket(const SPtr<ISession>& session, UPtr<PacketBody>&& packetBody) = 0;
};

template <typename TPacketType>
class PacketHandler : public IPacketHandler
{
    static_assert(std::is_base_of<PacketBody, TPacketType>::value, "TPacketType must inherit from PacketBody");
    using RawPacketHandler = std::function<bool(const SPtr<ISession>& session, UPtr<TPacketType>&& packetBody)>;

public:
    PacketHandler() = default;
    explicit PacketHandler(RawPacketHandler&& rawPacketHandler)
        : m_rawPacketHandler(rawPacketHandler)
    {
    }
    ~PacketHandler() override = default;
    PacketHandler(const PacketHandler&) = delete;
    PacketHandler(PacketHandler&&) = delete;
    PacketHandler& operator=(const PacketHandler&) = delete;
    PacketHandler& operator=(PacketHandler&&) = delete;

    bool ProcessPacket(const SPtr<ISession>& session, UPtr<PacketBody>&& packetBody) override;

private:
    RawPacketHandler m_rawPacketHandler;
};

template <typename TPacketType>
bool PacketHandler<TPacketType>::ProcessPacket(const SPtr<ISession>& session, UPtr<PacketBody>&& packetBody)
{
    std::unique_ptr<TPacketType> derivedPacketBody = static_cast<TPacketType*>(packetBody.release());
    if (nullptr == derivedPacketBody || nullptr == m_rawPacketHandler)
        return false;

    return m_rawPacketHandler(session, derivedPacketBody);
}