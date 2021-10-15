cmake_minimum_required(VERSION 3.21)
project(signalsafe VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

enable_testing()

option(ENABLE_COVERAGE "Enable code coverage flags." OFF)
option(ENABLE_ASAN "Enable address sanitizer flags." OFF)
option(ENABLE_UBSAN "Enable undefined behaviour sanitizer flags." OFF)

add_library(
    signalsafe
    source/file.cpp
)

target_include_directories(
    signalsafe
    PUBLIC
    include
)

target_compile_options(
    signalsafe
    PRIVATE
    -fPIC
    -Wall
    -Wextra
    -Wpedantic
    -Werror
)

if(ENABLE_COVERAGE)
    target_compile_options(
        signalsafe
        PRIVATE
        --coverage
    )
    target_link_libraries(
        signalsafe
        PRIVATE
        gcov
    )
endif()

if(ENABLE_ASAN OR ENABLE_UBSAN)
    target_compile_options(
        signalsafe
        PUBLIC
        -fno-omit-frame-pointer
    )
endif()

if(ENABLE_ASAN)
    target_compile_options(
        signalsafe
        PUBLIC
        -fsanitize=address
    )
    target_link_options(
        signalsafe
        PUBLIC
        -fsanitize=address
    )
endif()

if(ENABLE_UBSAN)
    target_compile_options(
        signalsafe
        PUBLIC
        -fsanitize=undefined
    )
    target_link_options(
        signalsafe
        PUBLIC
        -fsanitize=undefined
    )
endif()

add_subdirectory(test)

