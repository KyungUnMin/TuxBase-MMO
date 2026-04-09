#pragma once

struct PacketHeader
{
    UINT16 m_size;
    UINT16 m_id;
};

struct DummyPacket : public PacketHeader
{
    UINT32 m_sequence;
};
