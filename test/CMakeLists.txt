cmake_minimum_required(VERSION 3.21)
project(libsignalsafe-test LANGUAGES CXX)

include(FetchContent)
FetchContent_Declare(
    catch
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v2.13.7
)

FetchContent_MakeAvailable(catch)

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

add_test(
    NAME libsignalsafe-test
    COMMAND $<TARGET_FILE:libsignalsafe-test>
)

