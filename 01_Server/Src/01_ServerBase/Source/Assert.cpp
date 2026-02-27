#include "Assert.h"
#include "BackTrace.h"
#include <cstdlib>
#include <fmt/base.h>
#include <fmt/color.h>

namespace common {

void Assert(bool condition, const std::string &message, const char *fileName,
            int lineNumber) {
  if (!condition) {
    fmt::print(stderr, fmt::fg(fmt::color::red),
               "Assertion failed: {}\n ->{}:{}\n", message, fileName,
               lineNumber);
    PrintBackTrace();
    std::abort();
  }
}

void Crash(const std::string &message, const char *fileName, int lineNumber) {
  fmt::print(stderr, fmt::fg(fmt::color::red),
             "Crash Program! reason : {}\n ->{}:{}\n", message, fileName,
             lineNumber);
  PrintBackTrace();
  std::abort();
}

} // namespace common
