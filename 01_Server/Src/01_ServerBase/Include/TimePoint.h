#pragma once

inline std::uint64_t Tick()
{
    const auto now = std::chrono::steady_clock::now();
    return static_cast<std::uint64_t>(
        std::chrono::duration_cast<std::chrono::nanoseconds>(
            now.time_since_epoch())
            .count());
}

/**
 * @brief 경과 시간을 측정하는 스톱워치 클래스.
 * std::chrono::steady_clock 기반으로 나노초 단위의 정밀 시간을 측정합니다.
 */
class StopWatch
{
private:
    StopWatch(const StopWatch&) = delete;
    StopWatch(StopWatch&&) = delete;
    StopWatch& operator=(const StopWatch&) = delete;
    StopWatch& operator=(StopWatch&&) = delete;

public:
    StopWatch() = default;
    ~StopWatch() = default;

    void Start();
    void Stop();
    void Reset();

    std::uint64_t GetElapsedNanoSeconds() const;
    double GetElapsedSeconds() const;

private:
    std::uint64_t _startPoint = 0;
    std::uint64_t _endPoint = 0;
};