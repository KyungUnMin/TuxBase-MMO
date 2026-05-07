project(DummyClient LANGUAGES CXX)

file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/Include/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Include/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Include/*.inl"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cpp"
)

# Protobuf 설정
find_package(Protobuf CONFIG REQUIRED)
if(EXISTS "/root/share/Protocol")
    file(GLOB PROTO_FILES "/root/share/Protocol/*.proto")
else()
    file(GLOB PROTO_FILES "${CMAKE_CURRENT_SOURCE_DIR}/../../../03_Share/Protocol/*.proto")
endif()

if(PROTO_FILES)
    protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_FILES})
    list(APPEND SRC_FILES ${PROTO_SRCS} ${PROTO_HDRS})
endif()

add_executable(DummyClient ${SRC_FILES})
target_include_directories(DummyClient
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Include
        ${CMAKE_CURRENT_BINARY_DIR} # Protobuf 생성 헤더용
)

#ServerEngine 라이브러리 링크
target_link_libraries(DummyClient
    PRIVATE
        ServerEngine
        protobuf::libprotobuf
)
