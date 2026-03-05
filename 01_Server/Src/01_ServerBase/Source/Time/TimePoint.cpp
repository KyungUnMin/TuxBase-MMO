#include "Time/TimePoint.h"

namespace chrono = std::chrono;
using ChronoClock = chrono::steady_clock;
using ChronoTime = ChronoClock::time_point;
using ChronoNs = chrono::nanoseconds;

void StopWatch::Start()
{
    m_startPoint = GetNow();
}

void StopWatch::Stop()
{
    m_endPoint = GetNow();
}

UINT64 StopWatch::GetNow()
{
    const ChronoTime kNow = ChronoClock::now();
    const ChronoClock::duration kDuration = kNow.time_since_epoch();
    const ChronoNs kNanoSec = chrono::duration_cast<ChronoNs>(kDuration);
    return static_cast<UINT64>(kNanoSec.count());
}

UINT64 StopWatch::GetDurationNs() const
{
    // Stop이 호출되지 않았다면 현재 시간으로 계산
    if (0 < m_startPoint && 0 == m_endPoint)
    {
        return GetNow() - m_startPoint;
    }

    return m_endPoint - m_startPoint;
}

UINT64 StopWatch::GetDurationUs() const
{
    constexpr UINT64 kUsScale = 1'000;
    return GetDurationNs() / kUsScale;
}

UINT64 StopWatch::GetDurationMs() const
{
    constexpr UINT64 kMsScale = 1'000'000;
    return GetDurationNs() / kMsScale;
}

UINT64 StopWatch::GetDurationSec() const
{
    constexpr UINT64 kSecScale = 1'000'000'000;
    return GetDurationNs() / kSecScale;
}

void StopWatch::Reset()
{
    m_startPoint = 0;
    m_endPoint = 0;
}