project(ServerApp LANGUAGES CXX)

# ─────────────────────────────────────────────
# 소스 파일 수집
# ─────────────────────────────────────────────
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/Include/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Include/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Include/*.inl"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cpp"
)

# ─────────────────────────────────────────────
# 실행 파일 생성
# ─────────────────────────────────────────────
add_executable(GameServer ${SRC_FILES})

target_include_directories(GameServer
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Include
)

# ─────────────────────────────────────────────
# ServerEngine 라이브러리 링크 (Boost 전이 포함)
# ─────────────────────────────────────────────
target_link_libraries(GameServer
    PRIVATE
        ServerEngine
)
