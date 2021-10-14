cmake_minimum_required(VERSION 3.21)
project(libsignalsafe-test LANGUAGES CXX)

include(FetchContent)
FetchContent_Declare(
    catch
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v2.13.7
)

FetchContent_MakeAvailable(catch)

option(ENABLE_ASAN "Enable address sanitizer flags." OFF)
option(ENABLE_UBSAN "Enable undefined behaviour sanitizer flags." OFF)

add_executable(
    libsignalsafe-test
    source/libsignalsafe-test.cpp
    source/file-test.cpp
)

target_compile_options(
    libsignalsafe-test PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -Werror
)

target_include_directories(
    libsignalsafe-test PRIVATE
    include
)

target_link_libraries(
    libsignalsafe-test
    Catch2::Catch2
    libsignalsafe
)

if(ENABLE_ASAN OR ENABLE_UBSAN)
    target_compile_options(
        libsignalsafe
        PRIVATE
        -fno-omit-frame-pointer
    )
endif()

if(ENABLE_ASAN)
    target_compile_options(
        libsignalsafe
        PRIVATE
        -fsanitize=address
    )
endif()

if(ENABLE_UBSAN)
    target_compile_options(
        libsignalsafe
        PRIVATE
        -fsanitize=undefined
    )
endif()

add_test(
    NAME libsignalsafe-test
    COMMAND $<TARGET_FILE:libsignalsafe-test>
)

