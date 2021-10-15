#include "libsignalsafe-test.hpp"
#include "signalsafe/file.hpp"

#include <cstddef>
#include <cstring>
#include <filesystem>

using signalsafe::File;

namespace {
    enum class FileOpenState {
        Open,
        Closed
    };

    FileOpenState check_file_open_state(std::filesystem::path path) {
        const auto popenCommand = std::string("lsof ") + path.string();

        struct LinuxFilePtrRAII final {
            explicit LinuxFilePtrRAII(FILE* const _linuxFilePtr) : linuxFilePtr(_linuxFilePtr) { }
            ~LinuxFilePtrRAII() {
                if(linuxFilePtr != nullptr) {
                    REQUIRE(pclose(linuxFilePtr) != -1);
                }
            }

            FILE* const linuxFilePtr;
        };

        LinuxFilePtrRAII linuxFilePtrRAII(popen(popenCommand.c_str(), "r"));

        REQUIRE(linuxFilePtrRAII.linuxFilePtr != nullptr);

        std::array<char, 8> lineBuffer = { };

        fgets(lineBuffer.data(), lineBuffer.size(), linuxFilePtrRAII.linuxFilePtr);

        // no lsof output means nothing has it open
        return strnlen(lineBuffer.data(), lineBuffer.size()) == 0 ? FileOpenState::Closed : FileOpenState::Open;
    }

    std::filesystem::path get_temporary_file_path(const size_t lineNumber) {
        auto path = std::filesystem::temp_directory_path();
        path.concat("/libsignalsafe-test-test-" + std::to_string(lineNumber));

        std::filesystem::remove(path);

        REQUIRE(! std::filesystem::exists(path));

        return path;
    }
}

SCENARIO("signalsafe::file") {
    GIVEN("/dev/zero as a the target file path") {
        constexpr char targetFile[] = "/dev/zero";

        WHEN("open_existing is called with read-only permissions") {
            auto file = File::open_existing(targetFile, File::Permissions::ReadOnly);

            AND_WHEN("get_file_descriptor is called") {
                const auto fd = file.get_file_descriptor();

                THEN("it does not return -1") {
                    REQUIRE(fd != -1);
                }
            }

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
        const auto path = get_temporary_file_path(__LINE__);

        WHEN("create_and_open is called with write-only permissions") {
            auto file = File::create_and_open(path.c_str(), File::Permissions::WriteOnly);

            THEN("the file exists") {
                REQUIRE(std::filesystem::exists(path));
            }

            AND_GIVEN("some const non-zero bytes") {
                const std::array<std::byte, 5> data = {
                    std::byte{1},
                    std::byte{2},
                    std::byte{3},
                    std::byte{4},
                    std::byte{5}
                };

                WHEN("write is called") {
                    file.write(data);

                    AND_WHEN("the file is closed via .close()") {
                        file.close();

                        REQUIRE(check_file_open_state(path) == FileOpenState::Closed);

                        THEN("lsof reports the file isn't open") {

                            AND_WHEN("the file is reopened and read back into a buffer that previously contained all zeros") {
                                file = File::open_existing(path.c_str(), File::Permissions::ReadOnly);

                                std::array<std::byte, 5> target;
                                target.fill(std::byte{0});

                                file.read(target);

                                THEN("the read data matches what was written") {
                                    REQUIRE(data == target);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    GIVEN("a path to a file that doesn't currently exist") {
        const auto path = get_temporary_file_path(__LINE__); 

        WHEN("create_and_open is called with read-only permissions") {
            auto file = File::create_and_open(path.c_str(), File::Permissions::ReadOnly);

            THEN("the file is open") {
                REQUIRE(check_file_open_state(path) == FileOpenState::Open);

                AND_WHEN("a file is move constructed from this file") {
                    File newFile(std::move(file));

                    THEN("the file is still open") {
                        REQUIRE(check_file_open_state(path) == FileOpenState::Open);

                        AND_WHEN("the original file instance's destructor is called") {
                            file = File{};

                            THEN("the file is still open") {
                                REQUIRE(check_file_open_state(path) == FileOpenState::Open);

                                AND_WHEN("the moved-to instance's destructor is called") {
                                    newFile = File{};

                                    THEN("the file is no longer open") {
                                        REQUIRE(check_file_open_state(path) == FileOpenState::Closed);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    GIVEN("a default constructor file") {
        File file;

        WHEN("close is called") {
            const bool result = file.close();

            THEN("it returns false") {
                REQUIRE(result == false);
            }
        }
    }

    GIVEN("a default constructed file") {
        File file;

        WHEN("get_file_descriptor is called") {
            const auto fd = file.get_file_descriptor();

            THEN("it returns -1") {
                REQUIRE(fd == -1);
            }
        }
    }
}

