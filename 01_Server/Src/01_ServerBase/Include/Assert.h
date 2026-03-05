#pragma once

// NOLINTBEGIN(cppcoreguidelines-macro-usage)
#define CRASH(message, ...)                                            \
    ::Crash(fmt::format(message __VA_OPT__(, ) __VA_ARGS__), __FILE__, \
            __LINE__)

#ifdef ASSERT
#undef ASSERT
#endif

#ifdef DEBUG
#define ASSERT(condition, message, ...)                                 \
    ::Assert((condition),                                               \
             fmt::format(message __VA_OPT__(, ) __VA_ARGS__), __FILE__, \
             __LINE__)
#else
#define ASSERT(condition, message, ...) ((void)0)
#endif
// NOLINTEND(cppcoreguidelines-macro-usage)

void Assert(bool condition, const std::string& message, const char* fileName,
            int lineNumber);
void Crash(const std::string& message, const char* fileName, int lineNumber);