#pragma once
#include <span>

class SendBuffer
{
public:
    SendBuffer() = delete;
    explicit SendBuffer(UINT32 size);
    ~SendBuffer() = default;

    SendBuffer(const SendBuffer&) = delete;
    SendBuffer(SendBuffer&&) = delete;
    SendBuffer& operator=(const SendBuffer&) = delete;
    SendBuffer& operator=(SendBuffer&&) = delete;

    std::span<BYTE> GetWritableSpan() { return std::span<BYTE>(m_data); }
    std::span<const BYTE> GetSpan() const { return std::span<const BYTE>(m_data); }
    UINT32 GetSize() const { return static_cast<UINT32>(m_data.size()); }

private:
    std::vector<BYTE> m_data;
};