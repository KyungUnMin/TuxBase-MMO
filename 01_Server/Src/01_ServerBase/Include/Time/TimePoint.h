#pragma once

class StopWatch
{
public:
    StopWatch() = default;
    ~StopWatch() = default;

    StopWatch(const StopWatch&) = delete;
    StopWatch(StopWatch&&) = delete;
    StopWatch& operator=(const StopWatch&) = delete;
    StopWatch& operator=(StopWatch&&) = delete;

    void Start();
    void Stop();
    void Reset();

    UINT64 GetDurationNs() const;  // 나노초
    UINT64 GetDurationUs() const;  // 마이크로초
    UINT64 GetDurationMs() const;  // 밀리초
    UINT64 GetDurationSec() const; // 초

private:
    static UINT64 GetNow();

private:
    UINT64 m_startPoint = 0;
    UINT64 m_endPoint = 0;
};
