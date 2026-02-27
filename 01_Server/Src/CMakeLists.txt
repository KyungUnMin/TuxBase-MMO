cmake_minimum_required(VERSION 3.20)

# ─────────────────────────────────────────────
#사용자 설정 변수
# ─────────────────────────────────────────────
set(DEFAULT_BUILD_TYPE Debug)  #빌드 타입: Debug, Release


# ─────────────────────────────────────────────
#vcpkg 툴체인 파일이 없다면 vcpkg 툴체인 파일을 설정
# ─────────────────────────────────────────────
if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "vcpkg toolchain file")
endif()

project(TuxBaseMMO
    VERSION 0.1.0
    LANGUAGES CXX
)

# ─────────────────────────────────────────────
#컴파일 옵션 세팅
# ─────────────────────────────────────────────
set(CMAKE_CXX_STANDARD 20)              #C++20 사용
set(CMAKE_CXX_STANDARD_REQUIRED ON)     #C++20 필수
set(CMAKE_CXX_EXTENSIONS OFF)           #컴파일러 비표준 문법 사용하지 않음 (표준 C++만 사용 가능하게)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)   #컴파일러 명령어를 json 파일로 저장 (인텔리센스에 도움된대)

#build type 설정
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE ${DEFAULT_BUILD_TYPE})
endif()

# ─────────────────────────────────────────────
#하위 디렉토리에 있는 CMakeLists.txt 파일 읽어서 add_subdirectory
# ─────────────────────────────────────────────
file(GLOB children RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *)
list(SORT children) #add_subdirectory할때 순서 보장
foreach(child ${children})
    if(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${child}
       AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${child}/CMakeLists.txt)
        add_subdirectory(${child})
    endif()
endforeach()