project(ServerBase LANGUAGES CXX)

file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/Include/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Include/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Include/*.inl"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cpp"
)

#플랫폼별 BackTrace 파일 분리: GLOB에서 제외 후 조건부 추가
list(FILTER SRC_FILES EXCLUDE REGEX "BackTrace_(linux|win32)\\.cpp$")

#임시: RingBuffer 빌드 제외
list(FILTER SRC_FILES EXCLUDE REGEX "RingBuffer\\.(cpp|h)$")

add_library(ServerBase STATIC
    ${SRC_FILES}
)

if(WIN32)
    target_sources(ServerBase PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/Source/BackTrace_win32.cpp"
    )
    target_link_libraries(ServerBase PUBLIC dbghelp)
else()
    target_sources(ServerBase PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/Source/BackTrace_linux.cpp"
    )
endif()

target_include_directories(ServerBase
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Include
)

#fmt 라이브러리 링크
find_package(fmt CONFIG REQUIRED)
target_link_libraries(ServerBase PUBLIC fmt::fmt)

#PCH 설정
target_precompile_headers(ServerBase
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Include/pch.h
)
