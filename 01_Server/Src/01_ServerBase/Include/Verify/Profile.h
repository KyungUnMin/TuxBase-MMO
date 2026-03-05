#pragma once

#include "Time/TimePoint.h"

// NOLINTBEGIN(cppcoreguidelines-macro-usage)
#if defined(__PROFILE__)

#if defined(__GNUC__) || defined(__clang__)
#define TUXBASE_FUNCTION_NAME __PRETTY_FUNCTION__
#elif defined(_MSC_VER)
#define TUXBASE_FUNCTION_NAME __FUNCSIG__
#else
#define TUXBASE_FUNCTION_NAME __func__
#endif

#define FUNCTION_PROFILE() \
    ProfileSampler sampler##__LINE__(TUXBASE_FUNCTION_NAME);

#define BLOCK_PROFILE(name) \
    if (bool once = true)   \
        for (ProfileSampler sampler##__LINE__(name); once; once = false)

#define PROFILE_SAMPLE_START(fileName, bCachedProfile) \
    ProfileStart(fileName, bCachedProfile)
#define PROFILE_SAMPLE_END() ProfileEnd()

#else
#define FUNCTION_PROFILE()
#define BLOCK_PROFILE(name)
#define PROFILE_SAMPLE_START(fileName, bCachedProfile)
#define PROFILE_SAMPLE_END()
#endif
// NOLINTEND(cppcoreguidelines-macro-usage)

/**
 * @brief 프로파일링 샘플 데이터.
 */
struct SampleData
{
    const char* _blockName;
    double _elapsedTime;
};

void ProfileStart(const char* fileName, bool bCachedProfile);
void ProfileEnd();

/**
 * @brief RAII 기반 프로파일 샘플러.
 * 생성 시 타이머를 시작하고, 소멸 시 경과 시간을 기록합니다.
 */
class ProfileSampler
{
private:
    ProfileSampler() = delete;
    ProfileSampler(const ProfileSampler& other) = delete;
    ProfileSampler(ProfileSampler&& other) noexcept = delete;
    ProfileSampler& operator=(ProfileSampler&& other) noexcept = delete;
    ProfileSampler& operator=(const ProfileSampler& other) = delete;

public:
    template <typename T>
    ProfileSampler(const T*) = delete;
    // Not deletion string samplerName
    ProfileSampler(const char* samplerName);
    ~ProfileSampler();

private:
    const char* _samplerName;
    StopWatch _stopwatch;
};
