#pragma once

// ─────────────────────────────────────────────
// C++ 표준 라이브러리 — 기본 타입 & 유틸리티
// ─────────────────────────────────────────────
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <ctime>

// ─────────────────────────────────────────────
// C++ 표준 라이브러리 — 컨테이너
// ─────────────────────────────────────────────
#include <array>
#include <string>
#include <vector>

// ─────────────────────────────────────────────
// C++ 표준 라이브러리 — 알고리즘 & 유틸리티
// ─────────────────────────────────────────────
#include <algorithm>
#include <utility>

// ─────────────────────────────────────────────
// C++ 표준 라이브러리 — 시간
// ─────────────────────────────────────────────
#include <chrono>

// ─────────────────────────────────────────────
// C++ 표준 라이브러리 — I/O & 스트림
// ─────────────────────────────────────────────
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

// ─────────────────────────────────────────────
// 외부 라이브러리 — fmt
// ─────────────────────────────────────────────
#include <fmt/base.h>
#include <fmt/chrono.h>
#include <fmt/color.h>
#include <fmt/format.h>

// ─────────────────────────────────────────────
// 커스텀 타입 별칭
// ─────────────────────────────────────────────
using INT8 = signed char;
using INT16 = short;
using INT32 = int;
using INT64 = long long;

using UINT8 = unsigned char;
using UINT16 = unsigned short;
using UINT32 = unsigned int;
using UINT64 = unsigned long long;

using FLOAT32 = float;
using FLOAT64 = double;

using BYTE = unsigned char;
using BOOL = bool;
