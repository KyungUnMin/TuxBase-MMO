#pragma once
#include "DataStruct/RingBuffer/RingBuffer.h"
#include "EngineCommon/Packet.h"
#include <google/protobuf/arena.h>

class PacketSerializer
{
public:
    static bool Write(RingBuffer& buffer, UINT16 packetId, const PacketBody& packetBody)
    {
        if (nullptr == packetBody)
        {
            // TODO : Error Log
            return false;
        }

        const UINT32 bodySize = static_cast<UINT32>(packetBody->ByteSizeLong());
        const UINT32 totalSize = PacketHeader::kHeaderSize + bodySize;
        if (UINT16_MAX < totalSize)
        {
            // TODO : Error Log
            return false;
        }

        RingBufferWriter writer = buffer.CreateWriter(totalSize);
        if (false == writer.IsValid())
        {
            // TODO : Error Log. 버퍼 부족
            return false;
        }

        PacketHeader& header = writer.As<PacketHeader>();
        header.m_size = static_cast<UINT16>(totalSize);
        header.m_id = packetId;
        bool result = packetBody->SerializeToArray(writer.GetPtr(PacketHeader::kHeaderSize), bodySize);
        if (false == result)
        {
            // TODO : Error Log
            writer.GiveUp();
            return false;
        }

        writer.Commit();
        return true;
    }

    static bool PeekHeader(RingBuffer& buffer, PacketHeader& outHeader)
    {
        RingBufferReader reader = buffer.CreateReader(PacketHeader::kHeaderSize);
        if (false == reader.IsValid() || reader.GetSize() < PacketHeader::kHeaderSize)
        {
            if (reader.IsValid())
                reader.GiveUp();

            // 버퍼에 헤더만큼 안 찬 경우
            return false;
        }

        outHeader = reader.As<PacketHeader>();
        reader.GiveUp();
        return true;
    }

    template <typename TMessage>
    static bool Read(RingBuffer& buffer, const PacketHeader& header, TMessage& outMessage)
    {
        RingBufferReader reader = buffer.CreateReader(header.m_size);
        if (!reader.IsValid() || reader.GetSize() < header.m_size)
        {
            if (reader.IsValid())
                reader.GiveUp();
            return false;
        }

        const BYTE* bodyPtr = static_cast<const BYTE*>(reader.GetPtr()) + PacketHeader::kHeaderSize;
        const UINT32 bodySize = header.m_size - PacketHeader::kHeaderSize;

        bool success = outMessage.ParseFromArray(bodyPtr, bodySize);
        if (success)
        {
            reader.Commit();
        }
        else
        {
            reader.GiveUp();
        }
        return success;
    }

    template <typename TMessage>
    static TMessage* Read(RingBuffer& buffer, const PacketHeader& header, google::protobuf::Arena& arena)
    {
        TMessage* message = google::protobuf::Arena::CreateMessage<TMessage>(&arena);
        if (!Read(buffer, header, *message))
            return nullptr;
        return message;
    }
};
