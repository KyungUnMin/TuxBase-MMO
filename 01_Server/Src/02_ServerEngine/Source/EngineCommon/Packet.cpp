#include "EngineCommon/Packet.h"
#include "DataStruct/RecvBuffer.h"

/*-------------------------------------
  PacketParser
-------------------------------------*/

PacketHeader IPacketParser::ParsePacketHeader(const RecvBuffer& recvBuffer)
{
    PacketHeader packetHeader;
    if (recvBuffer.GetReadableSize() < PacketHeader::kHeaderSize)
    {
        // 패킷 헤더 크기만큼 데이터가 수신되지 않음
        return packetHeader;
    }

    std::memcpy(&packetHeader, recvBuffer.GetReadableSpan().data(), PacketHeader::kHeaderSize);
    if (recvBuffer.GetReadableSize() < packetHeader.m_size)
    {
        // 패킷 크기만큼 데이터가 수신되지 않음
        return packetHeader;
    }

    return packetHeader;
}