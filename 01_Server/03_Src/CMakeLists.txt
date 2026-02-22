cmake_minimum_required(VERSION 3.20)

# ─────────────────────────────────────────────
# vcpkg 툴체인 (환경변수 VCPKG_ROOT 자동 감지)
# ─────────────────────────────────────────────
if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "vcpkg toolchain file")
endif()

project(TuxBaseMMO
    VERSION 0.1.0
    LANGUAGES CXX
)

# ─────────────────────────────────────────────
# 글로벌 컴파일 옵션
# ─────────────────────────────────────────────
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# ─────────────────────────────────────────────
# 서브 프로젝트
# ─────────────────────────────────────────────
add_subdirectory(01_ServerEngine)
add_subdirectory(02_ServerApp)