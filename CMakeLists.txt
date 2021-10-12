cmake_minimum_required(VERSION 3.21)
project(libsignalsafe VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

enable_testing()

option(ENABLE_COVERAGE "Enable code coverage flags." OFF)
if(ENABLE_COVERAGE)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --coverage")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
    link_libraries(gcov)
endif()

add_library(
    libsignalsafe
    source/file.cpp
)

set_target_properties(
    libsignalsafe
    PROPERTIES
    PREFIX ""
)

target_include_directories(
    libsignalsafe
    PUBLIC
    include
)

target_compile_options(
    libsignalsafe
    PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -Werror
)

add_subdirectory(test)

