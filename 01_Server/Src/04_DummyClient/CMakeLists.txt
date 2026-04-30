project(DummyClient LANGUAGES CXX)

file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/Include/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Include/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Include/*.inl"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cpp"
)

add_executable(DummyClient ${SRC_FILES})
target_include_directories(DummyClient
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Include
)

#ServerEngine 라이브러리 링크
target_link_libraries(DummyClient
    PRIVATE
        ServerEngine
)
