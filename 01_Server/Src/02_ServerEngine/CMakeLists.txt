project(ServerEngine LANGUAGES CXX)

file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/Include/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Include/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Include/*.inl"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cpp"
)

add_library(ServerEngine STATIC
    ${SRC_FILES}
)

# include 참조 위치
target_include_directories(ServerEngine
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Include
)

#boost 라이브러리 추가
find_package(Boost CONFIG REQUIRED)
target_link_libraries(ServerEngine
    PUBLIC
        ServerBase
        Boost::headers
)

# PCH 설정
target_precompile_headers(ServerEngine
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Include/pch.h
)