#include <gtest/gtest.h>
#include "Time/StopWatch.h"
#include <thread>
#include <chrono>

class StopWatchTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_stopwatch.Reset();
    }

    StopWatch m_stopwatch;
};

// 시작 전에는 모든 기간이 0이어야 함
TEST_F(StopWatchTest, Init)
{
    EXPECT_EQ(m_stopwatch.GetDurationNs(), 0);
    EXPECT_EQ(m_stopwatch.GetDurationMs(), 0);
}

// Stop을 호출하지 않아도 실시간으로 시간이 흘러야 함
TEST_F(StopWatchTest, TimeCheck)
{
    m_stopwatch.Start();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    UINT64 duration = m_stopwatch.GetDurationMs();
    EXPECT_GE(duration, 10);
}

// Stop 호출 후에는 시간이 고정되어야 함
TEST_F(StopWatchTest, StopTest)
{
    m_stopwatch.Start();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    m_stopwatch.Stop();

    UINT64 durationAtStop = m_stopwatch.GetDurationMs();
    std::this_thread::sleep_for(std::chrono::milliseconds(20));

    EXPECT_EQ(m_stopwatch.GetDurationMs(), durationAtStop);
}

// Reset 호출 시 모든 기록이 지워져야 함
TEST_F(StopWatchTest, ResetTest)
{
    m_stopwatch.Start();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    m_stopwatch.Stop();

    m_stopwatch.Reset();
    EXPECT_EQ(m_stopwatch.GetDurationNs(), 0);
}

// Ns, Us, Ms, Sec 변환이 올바른지 확인
TEST_F(StopWatchTest, ConversionTest)
{
    m_stopwatch.Start();
    std::this_thread::sleep_for(std::chrono::milliseconds(1100));
    m_stopwatch.Stop();

    UINT64 ns = m_stopwatch.GetDurationNs();
    UINT64 us = m_stopwatch.GetDurationUs();
    UINT64 ms = m_stopwatch.GetDurationMs();
    UINT64 sec = m_stopwatch.GetDurationSec();

    EXPECT_GE(sec, 1);
    EXPECT_NEAR(static_cast<double>(ms), static_cast<double>(ns) / 1000000.0, 1.0);
    EXPECT_NEAR(static_cast<double>(us), static_cast<double>(ns) / 1000.0, 1.0);
    EXPECT_EQ(sec, ms / 1000);
}
