#pragma once

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

template <typename T>
using SPtr = std::shared_ptr<T>;

template <typename T>
using UPtr = std::unique_ptr<T>;

template <typename T>
using WPtr = std::weak_ptr<T>;