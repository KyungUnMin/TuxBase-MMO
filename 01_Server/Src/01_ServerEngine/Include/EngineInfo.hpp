#pragma once

#include <cstdint>

namespace TuxBase {
/// <summary>
/// ServerEngine 라이브러리 버전 정보 및 초기화 유틸리티.
/// </summary>
struct EngineInfo {
  static constexpr uint32_t VERSION_MAJOR = 0;
  static constexpr uint32_t VERSION_MINOR = 1;
  static constexpr uint32_t VERSION_PATCH = 0;
};
} // namespace TuxBase
