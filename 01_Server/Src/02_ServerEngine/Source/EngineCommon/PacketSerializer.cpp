#include "EngineCommon/PacketSerializer.h"
#include "Verify/Assert.h"
#include <memory>

UPtr<Packet> PacketSerializer::DeserializePacket(const RecvBuffer& recvBuffer)
{
    PacketHeader packetHeader = PeekRecvPacketHeader(recvBuffer);
    if (false == packetHeader.IsValid())
    {
        // 패킷 헤더만큼 데이터가 수신되지 않음
        return nullptr;
    }

    if (recvBuffer.GetReadableSize() < packetHeader.m_size)
    {
        // 패킷 크기만큼 데이터가 수신되지 않음
        return nullptr;
    }

    Iterator findIter = m_packerDeserializeHandlers.find(packetHeader.m_id);
    ASSERT(findIter != m_packerDeserializeHandlers.end(), "Packet id(%d) handler is not exists", packetId);

    const DeserialzeHandler& deserialzeHandler = findIter->second;
    UPtr<PacketBody> packetBody = deserialzeHandler(packetHeader.m_id);
    ASSERT(nullptr != packetBody, "Packet id(%d) handler is invalid", packetId);

    return std::make_unique<Packet>(packetHeader, std::move(packetBody));
}

PacketHeader PacketSerializer::PeekRecvPacketHeader(const RecvBuffer& recvBuffer)
{
    PacketHeader packetHeader;
    if (recvBuffer.GetReadableSize() < PacketHeader::kHeaderSize)
    {
        // 패킷 헤더 크기만큼 데이터가 수신되지 않음
        return packetHeader;
    }

    std::memcpy(&packetHeader, recvBuffer.GetReadableSpan().data(), PacketHeader::kHeaderSize);
    return packetHeader;
}

// std::shared_ptr<const SendBuffer> PacketSerializer::Serialize(UINT16 packetId, const PacketBody& packetBody)
// {
//     const UINT32 bodySize = static_cast<UINT32>(packetBody.ByteSizeLong());
//     const UINT32 totalSize = PacketHeader::kHeaderSize + bodySize;

//     if (PacketHeader::kMaxPacketSize < totalSize)
//     {
//         // TODO : Error Log
//         return nullptr;
//     }

//     std::shared_ptr<SendBuffer> sendBuffer = std::make_shared<SendBuffer>(totalSize);
//     BYTE* ptr = sendBuffer->GetWritableSpan().data();

//     PacketHeader header;
//     header.m_size = static_cast<UINT16>(totalSize);
//     header.m_id = packetId;
//     std::memcpy(ptr, &header, PacketHeader::kHeaderSize);

//     bool result = packetBody.SerializeToArray(ptr + PacketHeader::kHeaderSize, static_cast<int>(bodySize));
//     if (false == result)
//     {
//         // TODO : Error Log
//         return nullptr;
//     }

//     return sendBuffer;
// }

// bool PacketSerializer::Read(RecvBuffer& buffer, const PacketHeader& header, PacketBody& outMessage)
// {
//     const BYTE* bodyPtr = buffer.GetReadableSpan().data() + PacketHeader::kHeaderSize;
//     const UINT32 bodySize = header.m_size - PacketHeader::kHeaderSize;

//     bool success = outMessage.ParseFromArray(bodyPtr, static_cast<int>(bodySize));
//     if (success)
//     {
//         buffer.Consume(header.m_size);
//     }
//     return success;
// }