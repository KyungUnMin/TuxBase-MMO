#pragma once

#include <fmt/format.h>
#include <string>

// NOLINTBEGIN(cppcoreguidelines-macro-usage)
#define CRASH(message, ...)                                                    \
  ::common::Crash(fmt::format(message __VA_OPT__(, ) __VA_ARGS__), __FILE__,   \
                  __LINE__)

#ifdef ASSERT
#undef ASSERT
#endif

#ifdef DEBUG
#define ASSERT(condition, message, ...)                                        \
  ::common::Assert((condition),                                                \
                   fmt::format(message __VA_OPT__(, ) __VA_ARGS__), __FILE__,  \
                   __LINE__)
#else
#define ASSERT(condition, message, ...) ((void)0)
#endif
// NOLINTEND(cppcoreguidelines-macro-usage)

namespace common {

void Assert(bool condition, const std::string &message, const char *fileName,
            int lineNumber);
void Crash(const std::string &message, const char *fileName, int lineNumber);

} // namespace common