#pragma once
#include "EngineCommon/Packet.h"

class IPacketParser
{
public:
    IPacketParser() = default;
    virtual ~IPacketParser() = default;
    IPacketParser(const IPacketParser&) = delete;
    IPacketParser(IPacketParser&&) = delete;
    IPacketParser& operator=(const IPacketParser&) = delete;
    IPacketParser& operator=(IPacketParser&&) = delete;

    static PacketHeader ParsePacketHeader(const class RecvBuffer& recvBuffer);
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