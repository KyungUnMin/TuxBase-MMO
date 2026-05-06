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
    "SignalHandler_(linux|window)\\.cpp$"
    "Profile\\.(cpp|h)$"
)
foreach(IGNORE_SRC_FILE IN LISTS IGNORE_SRC_FILES)
    list(FILTER SRC_FILES EXCLUDE REGEX "${IGNORE_SRC_FILE}")
endforeach()

# 운영체제 별로 다르게 처리하는 소스 파일 추가
if(WIN32)
    list(APPEND SRC_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/Source/Verify/BackTrace_win32.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/Source/Verify/SignalHandler_window.cpp"
    )
elseif(UNIX)
    list(APPEND SRC_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/Source/Verify/BackTrace_linux.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/Source/Verify/SignalHandler_linux.cpp"
    )
endif()

# Protobuf 설정
find_package(Protobuf CONFIG REQUIRED)

# 컨테이너 환경의 마운트 경로(/root/share)를 기본으로 탐색하되 없으면 호스트 경로 탐색
if(EXISTS "/root/share/Protocol")
    file(GLOB PROTO_FILES "/root/share/Protocol/*.proto")
else()
    file(GLOB PROTO_FILES "${CMAKE_CURRENT_SOURCE_DIR}/../../../03_Share/Protocol/*.proto")
endif()

if(PROTO_FILES)
    protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_FILES})
    list(APPEND SRC_FILES ${PROTO_SRCS} ${PROTO_HDRS})
endif()

add_library(ServerBase STATIC
    ${SRC_FILES}
)

# include 참조 위치
target_include_directories(ServerBase
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Include
        ${CMAKE_CURRENT_BINARY_DIR} # Protobuf가 생성한 .pb.h 파일(build 폴더 내)을 include 하기 위함
)

# vcpkg에 설치한 라이브러리 링크
find_package(fmt CONFIG REQUIRED)
target_link_libraries(ServerBase PUBLIC fmt::fmt protobuf::libprotobuf)

# 플랫폼별 링크 라이브러리
if(WIN32)
    target_link_libraries(ServerBase PUBLIC dbghelp)
endif()

# PCH 설정
target_precompile_headers(ServerBase
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Include/pch.h
)
