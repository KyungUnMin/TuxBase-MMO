#include <gtest/gtest.h>
#include "TimePoint.h"

TEST(StopWatchTest, InitialStateIsZero)
{
    common::StopWatch sw;
    EXPECT_EQ(sw.GetElapsedNanoSeconds(), 0);
}

TEST(StopWatchTest, MeasuresElapsedTime)
{
    common::StopWatch sw;
    sw.Start();

    // 간단한 지연
    volatile int sum = 0;
    for (int i = 0; i < 100000; ++i)
    {
        sum += i;
    }

    sw.Stop();
    EXPECT_GT(sw.GetElapsedNanoSeconds(), 0);
    EXPECT_GT(sw.GetElapsedSeconds(), 0.0);
}

TEST(StopWatchTest, ResetClearsTime)
{
    common::StopWatch sw;
    sw.Start();
    sw.Stop();
    sw.Reset();
    EXPECT_EQ(sw.GetElapsedNanoSeconds(), 0);
}
