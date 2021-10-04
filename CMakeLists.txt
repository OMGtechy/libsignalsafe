cmake_minimum_required(VERSION 3.21)
project(libsignalsafe VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

enable_testing()

add_library(
    libsignalsafe
    source/file.cpp
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

