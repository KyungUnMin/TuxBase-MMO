project(ServerEngine LANGUAGES CXX)

# ─────────────────────────────────────────────
# 소스 파일 수집
# ─────────────────────────────────────────────
file(GLOB_RECURSE ENGINE_HEADERS CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/Include/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Include/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Include/*.inl"
)

file(GLOB_RECURSE ENGINE_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cpp"
)

# ─────────────────────────────────────────────
# 정적 라이브러리 생성
# ─────────────────────────────────────────────
add_library(ServerEngine STATIC
    ${ENGINE_HEADERS}
    ${ENGINE_SOURCES}
)

target_include_directories(ServerEngine
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Include
)

# ─────────────────────────────────────────────
# Boost 의존성
# ─────────────────────────────────────────────
find_package(Boost REQUIRED COMPONENTS system)

target_link_libraries(ServerEngine
    PUBLIC
        Boost::system
)
