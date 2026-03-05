#include "Time/TimePoint.h"

void StopWatch::Start()
{
    _startPoint = Tick();
}

void StopWatch::Stop()
{
    _endPoint = Tick();
}

void StopWatch::Reset()
{
    _startPoint = 0;
    _endPoint = 0;
}

std::uint64_t StopWatch::GetElapsedNanoSeconds() const
{
    return _endPoint - _startPoint;
}

double StopWatch::GetElapsedSeconds() const
{
    constexpr double kNanoSecondsScale = 1'000'000'000.0;
    return static_cast<double>(GetElapsedNanoSeconds()) / kNanoSecondsScale;
}
