project(ServerBase LANGUAGES CXX)

file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/Include/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Include/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Include/*.inl"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cpp"
)

# 빌드 제외 파일 목록
set(IGNORE_SRC_FILES
    "BackTrace_(linux|win32)\\.cpp$"
    "RingBuffer\\.(cpp|h)$"
    "Profile\\.(cpp|h)$"
)
foreach(IGNORE_SRC_FILE IN LISTS IGNORE_SRC_FILES)
    list(FILTER SRC_FILES EXCLUDE REGEX "${IGNORE_SRC_FILE}")
endforeach()

# 운영체제 별로 다르게 처리하는 소스 파일 추가
if(WIN32)
    list(APPEND SRC_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/Source/Verify/BackTrace_win32.cpp"
    )
elseif(UNIX)
    list(APPEND SRC_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/Source/Verify/BackTrace_linux.cpp"
    )
endif()

add_library(ServerBase STATIC
    ${SRC_FILES}
)

# include 참조 위치
target_include_directories(ServerBase
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Include
)

# vcpkg에 설치한 라이브러리 링크
find_package(fmt CONFIG REQUIRED)
target_link_libraries(ServerBase PUBLIC fmt::fmt)

# 플랫폼별 링크 라이브러리
if(WIN32)
    target_link_libraries(ServerBase PUBLIC dbghelp)
endif()

# PCH 설정
target_precompile_headers(ServerBase
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Include/pch.h
)
