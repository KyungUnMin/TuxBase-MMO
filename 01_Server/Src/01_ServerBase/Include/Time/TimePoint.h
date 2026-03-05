#pragma once

inline std::uint64_t Tick()
{
    const auto now = std::chrono::steady_clock::now();
    return static_cast<std::uint64_t>(
        std::chrono::duration_cast<std::chrono::nanoseconds>(
            now.time_since_epoch())
            .count());
}

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
