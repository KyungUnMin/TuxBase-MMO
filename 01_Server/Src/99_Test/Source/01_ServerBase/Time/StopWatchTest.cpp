#include <gtest/gtest.h>
#include "Time/StopWatch.h"
#include <thread>
#include <chrono>

/**
 * @brief StopWatch 클래스에 대한 유닛 테스트.
 */
class StopWatchTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_stopwatch.Reset();
    }

    StopWatch m_stopwatch;
};

// 1. 초기 상태 테스트: 시작 전에는 모든 기간이 0이어야 함
TEST_F(StopWatchTest, InitialStateIsZero)
{
    EXPECT_EQ(m_stopwatch.GetDurationNs(), 0);
    EXPECT_EQ(m_stopwatch.GetDurationMs(), 0);
}

// 2. 실행 중 측정 테스트: Stop을 호출하지 않아도 실시간으로 시간이 흘러야 함
TEST_F(StopWatchTest, DurationIncreasesWhileRunning)
{
    m_stopwatch.Start();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    UINT64 duration = m_stopwatch.GetDurationMs();
    EXPECT_GE(duration, 10);
}

// 3. 정지 테스트: Stop 호출 후에는 시간이 고정되어야 함
TEST_F(StopWatchTest, DurationIsFixedAfterStop)
{
    m_stopwatch.Start();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    m_stopwatch.Stop();

    UINT64 durationAtStop = m_stopwatch.GetDurationMs();
    std::this_thread::sleep_for(std::chrono::milliseconds(20));

    EXPECT_EQ(m_stopwatch.GetDurationMs(), durationAtStop);
}

// 4. 리셋 테스트: Reset 호출 시 모든 기록이 지워져야 함
TEST_F(StopWatchTest, ResetClearsDuration)
{
    m_stopwatch.Start();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    m_stopwatch.Stop();

    m_stopwatch.Reset();
    EXPECT_EQ(m_stopwatch.GetDurationNs(), 0);
}

// 5. 단위 변환 테스트: Ns, Us, Ms, Sec 변환이 올바른지 확인
TEST_F(StopWatchTest, UnitConversion)
{
    m_stopwatch.Start();
    // 약 1.1초 대기
    std::this_thread::sleep_for(std::chrono::milliseconds(1100));
    m_stopwatch.Stop();

    UINT64 ns = m_stopwatch.GetDurationNs();
    UINT64 us = m_stopwatch.GetDurationUs();
    UINT64 ms = m_stopwatch.GetDurationMs();
    UINT64 sec = m_stopwatch.GetDurationSec();

    // 오차 범위를 고려한 대략적인 검증
    EXPECT_GE(sec, 1);
    EXPECT_NEAR(static_cast<double>(ms), static_cast<double>(ns) / 1'000'000.0, 1.0);
    EXPECT_NEAR(static_cast<double>(us), static_cast<double>(ns) / 1'000.0, 1.0);
    EXPECT_EQ(sec, ms / 1000);
}
