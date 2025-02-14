cmake_minimum_required(VERSION 3.25.0)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(EXISTS "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake")
    set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake")
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

project(psb-sockutils VERSION 0.0.1 LANGUAGES CXX)

option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
option(BUILD_TESTING "Whether to enable tests" ${PROJECT_IS_TOP_LEVEL})
option(INSTALL_SOCKUTILS "Whether to enable install targets" ${PROJECT_IS_TOP_LEVEL})
option(ENABLE_MAINTAINER_MODE "Enable maintainer mode" ON)
option(USE_CLANG_TIDY "Use clang-tidy" OFF)

include(build_types)
include(tools)
include(coverage)

find_package(opentelemetry-cpp CONFIG REQUIRED)

add_subdirectory(src)

if(BUILD_TESTING)
    find_package(GTest CONFIG REQUIRED)
    find_package(Microsoft.GSL CONFIG REQUIRED)

    include(CTest)
    enable_testing()
    add_subdirectory(test)
endif()
