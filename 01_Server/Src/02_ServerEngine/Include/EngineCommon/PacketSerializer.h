#pragma once
#include "DataStruct/RecvBuffer.h"
#include "DataStruct/SendBuffer.h"
#include "EngineCommon/Packet.h"
#include <memory>

class PacketSerializer
{
public:
    static std::shared_ptr<const SendBuffer> Serialize(UINT16 packetId, const PacketBody& packetBody);
    static bool PeekHeader(const RecvBuffer& buffer, PacketHeader& outHeader);
    static bool IsValidHeader(const PacketHeader& header);
    static bool Read(RecvBuffer& buffer, const PacketHeader& header, PacketBody& outMessage);

    template <typename TMessage>
    static std::unique_ptr<TMessage> Read(RecvBuffer& buffer, const PacketHeader& header)
    {
        std::unique_ptr<TMessage> message = std::make_unique<TMessage>();
        if (!Read(buffer, header, *message))
            return nullptr;

        return message;
    }
};