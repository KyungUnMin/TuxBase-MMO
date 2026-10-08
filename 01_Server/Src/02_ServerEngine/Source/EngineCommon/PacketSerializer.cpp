#include "EngineCommon/PacketSerializer.h"

std::shared_ptr<const SendBuffer> PacketSerializer::Serialize(UINT16 packetId, const PacketBody& packetBody)
{
    const UINT32 bodySize = static_cast<UINT32>(packetBody.ByteSizeLong());
    const UINT32 totalSize = PacketHeader::kHeaderSize + bodySize;

    if (PacketHeader::kMaxPacketSize < totalSize)
    {
        // TODO : Error Log
        return nullptr;
    }

    std::shared_ptr<SendBuffer> sendBuffer = std::make_shared<SendBuffer>(totalSize);
    BYTE* ptr = sendBuffer->GetWritableSpan().data();

    PacketHeader header;
    header.m_size = static_cast<UINT16>(totalSize);
    header.m_id = packetId;
    std::memcpy(ptr, &header, PacketHeader::kHeaderSize);

    bool result = packetBody.SerializeToArray(ptr + PacketHeader::kHeaderSize, static_cast<int>(bodySize));
    if (false == result)
    {
        // TODO : Error Log
        return nullptr;
    }

    return sendBuffer;
}

bool PacketSerializer::PeekHeader(const RecvBuffer& buffer, PacketHeader& outHeader)
{
    if (buffer.GetReadableSize() < PacketHeader::kHeaderSize)
    {
        // 버퍼에 헤더만큼 안 찬 경우
        return false;
    }

    std::memcpy(&outHeader, buffer.GetReadableSpan().data(), PacketHeader::kHeaderSize);
    return true;
}

bool PacketSerializer::IsValidHeader(const PacketHeader& header)
{
    return (PacketHeader::kHeaderSize <= header.m_size) && (header.m_size <= PacketHeader::kMaxPacketSize);
}

bool PacketSerializer::Read(RecvBuffer& buffer, const PacketHeader& header, PacketBody& outMessage)
{
    if (false == IsValidHeader(header) || buffer.GetReadableSize() < header.m_size)
    {
        return false;
    }

    const BYTE* bodyPtr = buffer.GetReadableSpan().data() + PacketHeader::kHeaderSize;
    const UINT32 bodySize = header.m_size - PacketHeader::kHeaderSize;

    bool success = outMessage.ParseFromArray(bodyPtr, static_cast<int>(bodySize));
    if (success)
    {
        buffer.Consume(header.m_size);
    }
    return success;
}