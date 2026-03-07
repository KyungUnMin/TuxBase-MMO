project(ServerApp LANGUAGES CXX)

file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/Include/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Include/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Include/*.inl"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cpp"
)

add_executable(GameServer ${SRC_FILES})
target_include_directories(GameServer
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Include
)

#ServerEngine 라이브러리 링크
target_link_libraries(GameServer
    PRIVATE
        ServerEngine
)
