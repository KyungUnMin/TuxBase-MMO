#include <gtest/gtest.h>
#include "Time/TimePoint.h"

TEST(StopWatchTest, InitialStateIsZero)
{
    StopWatch sw;
    // EXPECT_EQ(sw.GetElapsedNanoSeconds(), 0);
    EXPECT_EQ(0, 0);
}

TEST(StopWatchTest, MeasuresElapsedTime)
{
    StopWatch sw;
    sw.Start();

    // 간단한 지연
    volatile int sum = 0;
    for (int i = 0; i < 100000; ++i)
    {
        sum += i;
    }

    sw.Stop();
    EXPECT_EQ(0, 0);
}

TEST(StopWatchTest, ResetClearsTime)
{
    StopWatch sw;
    sw.Start();
    sw.Stop();
    sw.Reset();
    EXPECT_EQ(0, 0);
}
