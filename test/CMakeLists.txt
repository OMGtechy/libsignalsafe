cmake_minimum_required(VERSION 3.16)
project(signalsafe-test LANGUAGES CXX)

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
    signalsafe-test
    source/signalsafe-test.cpp
    source/file-test.cpp
    source/memory-test.cpp
    source/string-test.cpp
    source/string-test-alt.cpp
    source/time-test.cpp
)

target_compile_options(
    signalsafe-test PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -Werror
)

target_include_directories(
    signalsafe-test PRIVATE
    include
)

target_link_libraries(
    signalsafe-test
    Catch2::Catch2
    signalsafe
)

if(ENABLE_ASAN OR ENABLE_UBSAN)
    target_compile_options(
        signalsafe-test
        PRIVATE
        -fno-omit-frame-pointer
    )
endif()

if(ENABLE_ASAN)
    target_compile_options(
        signalsafe-test
        PRIVATE
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
        signalsafe-test
        PRIVATE
        -fsanitize=undefined
    )
    target_link_options(
        signalsafe
        PUBLIC
        -fsanitize=undefined
    )
endif()

add_test(
    NAME signalsafe-test
    COMMAND $<TARGET_FILE:signalsafe-test>
)

