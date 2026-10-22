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

    UPtr<PacketBody> ParsePacketBody(std::span<const BYTE> rawPacketBodyData) const override
    {
        UPtr<TPacketType> packetBody = std::make_unique<TPacketType>();
        if (nullptr == packetBody)
            return nullptr;

        if (false == packetBody->ParseFromArray(rawPacketBodyData.data(), rawPacketBodyData.size_bytes()))
            return nullptr;

        return std::move(packetBody);
    }
};

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

public:
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

    bool ProcessPacket(const SPtr<ISession>& session, UPtr<PacketBody>&& packetBody) override
    {
        std::unique_ptr<TPacketType> derivedPacketBody = static_cast<TPacketType*>(packetBody.release());
        if (nullptr == derivedPacketBody || nullptr == m_rawPacketHandler)
            return false;

        return m_rawPacketHandler(session, derivedPacketBody);
    }

private:
    RawPacketHandler m_rawPacketHandler;
};

/*-------------------------------------
            PacketDispatcher
-------------------------------------*/

class PacketDispatcher
{
    struct PacketProcessGroup
    {
        UPtr<IPacketParser> parser;
        UPtr<IPacketHandler> handler;
    };

    using Stl_HashMap = std::unordered_map<UINT16, PacketProcessGroup>;
    using Const_Iterator = Stl_HashMap::const_iterator;

public:
    PacketDispatcher() = default;
    ~PacketDispatcher() = default;
    PacketDispatcher(const PacketDispatcher&) = delete;
    PacketDispatcher(PacketDispatcher&&) = delete;
    PacketDispatcher& operator=(const PacketDispatcher&) = delete;
    PacketDispatcher& operator=(PacketDispatcher&&) = delete;

    template <typename PacketType>
    bool RegisterPacket(UINT16 packetId, PacketHandler<PacketType>::RawPacketHandler&& rawPacketHandler)
    {
        UPtr<IPacketParser> packetParser = std::make_unique<PacketParser<PacketType>>();
        UPtr<IPacketHandler> packetHandler = std::make_unique<PacketHandler<PacketType>>(std::move(rawPacketHandler));

        if (true == m_packetProcessGroup.contains(packetId))
            return false;

        return m_packetProcessGroup.insert(std::make_pair(packetId, PacketProcessGroup{std::move(packetParser), std::move(packetHandler)})).second;
    }
    // bool ProcessPacket(const SPtr<ISession>& session, const UPtr<Packet>& packet);

private:
    Stl_HashMap m_packetProcessGroup;
};
