#pragma once
#include "EngineCommon/Packet.h"

class ISession;

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
