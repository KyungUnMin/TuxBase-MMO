project(Tests LANGUAGES CXX)

file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cpp"
)

add_executable(Tests ${SRC_FILES})

target_include_directories(Tests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

#GTest 라이브러리 링크
find_package(GTest CONFIG REQUIRED)
target_link_libraries(Tests
    PRIVATE
        ServerEngine
        GTest::gtest
        GTest::gtest_main
)
