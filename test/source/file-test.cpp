#include "libsignalsafe-test.hpp"
#include "signalsafe/file.hpp"

#include <cstddef>
#include <filesystem>

SCENARIO("signalsafe::file") {
    GIVEN("/dev/zero as a the target file path") {
        constexpr char targetFile[] = "/dev/zero";

        WHEN("open_existing is called") {
            auto file = signalsafe::File::open_existing(targetFile);

            AND_GIVEN("a 12-byte target array initialised to 1") {
                std::array<std::byte, 12> target;
                target.fill(std::byte{1});

                WHEN("read is called with the middle 10 bytes of the target") {
                    file.read({target.begin() + 1, 10});

                    THEN("the first and last bytes are unchanged") {
                        REQUIRE(target.front() == std::byte{1});
                        REQUIRE(target.back()  == std::byte{1});
                    }

                    THEN("the middle 10 zero-bytes of the target are 0") {
                        REQUIRE(target[1]  == std::byte{0});
                        REQUIRE(target[2]  == std::byte{0});
                        REQUIRE(target[3]  == std::byte{0});
                        REQUIRE(target[4]  == std::byte{0});
                        REQUIRE(target[5]  == std::byte{0});
                        REQUIRE(target[6]  == std::byte{0});
                        REQUIRE(target[7]  == std::byte{0});
                        REQUIRE(target[8]  == std::byte{0});
                        REQUIRE(target[9]  == std::byte{0});
                        REQUIRE(target[10] == std::byte{0});
                    }
                }
            }
        }
    }

    GIVEN("a path to a file that doesn't currently exist") {
        auto path = std::filesystem::temp_directory_path();
        path.concat("/create_and_open_test");

        std::filesystem::remove(path);

        REQUIRE(! std::filesystem::exists(path));

        WHEN("create_and_open is called with it") {
            auto file = signalsafe::File::create_and_open(path.c_str());

            THEN("the file exists") {
                REQUIRE(std::filesystem::exists(path));
            }
        }
    }
}

