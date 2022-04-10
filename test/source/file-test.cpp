#include "signalsafe-test.hpp"
#include <signalsafe/file.hpp>

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

        const auto fgetsReturn = fgets(lineBuffer.data(), lineBuffer.size(), linuxFilePtrRAII.linuxFilePtr);
        const auto strLen = strnlen(lineBuffer.data(), lineBuffer.size());
        const auto readEmptyString = strLen == 0;

        // fgets returns NULL when either:
        // a) there's an error
        // b) there was no data to read
        // the latter is fine, the former is not
        REQUIRE((
            (fgetsReturn == lineBuffer.data() && !readEmptyString)
         || (fgetsReturn == NULL && readEmptyString)
        ));

        // no lsof output means nothing has it open
        return readEmptyString ? FileOpenState::Closed : FileOpenState::Open;
    }

    std::filesystem::path get_temporary_file_path(const size_t lineNumber) {
        auto path = std::filesystem::temp_directory_path();
        path.concat("/signalsafe-test-" + std::to_string(lineNumber));

        std::filesystem::remove(path);

        REQUIRE(! std::filesystem::exists(path));

        return path;
    }

    bool is_fd_valid(int fd) {
        return fcntl(fd, F_GETFD) != -1;
    }
}

SCENARIO("signalsafe::File") {
    GIVEN("a c-string") {
        const char str[] = "Testing STDOUT works\n";

        WHEN("it is written to standard_output") {
            const auto bytesWritten = signalsafe::standard_output().write(str);

            THEN("the number of bytes written is correct") {
                REQUIRE(bytesWritten == sizeof(str));
            }
        }
    }

    GIVEN("a c-string") {
        const char str[] = "Testing STDERR works\n";

        WHEN("it is written to standard_error") {
            const auto bytesWritten = signalsafe::standard_error().write(str);

            THEN("the number of bytes written is correct") {
                REQUIRE(bytesWritten == sizeof(str));
            }
        }
    }

    WHEN("create_and_open_temporary is called") {
        auto file = std::make_unique<File>(File::create_and_open_temporary());

        AND_WHEN("get_file_descriptor is called") {
            const auto fd = file->get_file_descriptor();

            THEN("it is a valid file descriptor") {
                REQUIRE(is_fd_valid(fd));
            }

            AND_WHEN("set_destroy_action is called with Nothing") {
                file->set_destroy_action(File::DestroyAction::Nothing);

                AND_WHEN("the file is destroyed") {
                    file = nullptr;

                    THEN("the file descriptor is still valid") {
                        REQUIRE(is_fd_valid(fd));
                    }

                    AND_WHEN("a new file instance is created using that file descriptor") {
                        file = std::make_unique<File>(File::from_file_descriptor(fd));

                        AND_WHEN("that file is destroyed") {
                            file = nullptr;

                            THEN("the file descriptor is no longer valid") {
                                REQUIRE(! is_fd_valid(fd));
                            }
                        }
                    }
                }
            }
        }
    }

    GIVEN("/dev/zero as a the target file path") {
        const std::string targetFile = "/dev/zero";

        WHEN("open_existing is called with read-only permissions") {
            auto file = File::open_existing(targetFile, File::Permissions::ReadOnly);

            AND_WHEN("get_path is called") {
                const auto path = file.get_path();

                THEN("it matches the path provided") {
                    REQUIRE(path == targetFile);
                }
            }

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

                    AND_WHEN("remove is called") {
                        const auto removed = file.remove();

                        THEN("it fails (because it's /dev/zero)") {
                            REQUIRE(! removed);
                        }

                        AND_WHEN("get_path is called") {
                            const auto returnedPath = file.get_path();

                            THEN("it still contains /dev/zero") {
                                REQUIRE(returnedPath == targetFile);
                            }
                        }
                    }
                }
            }
        }
    }

    GIVEN("a path to a file that doesn't currently exist") {
        const auto path = get_temporary_file_path(__LINE__);

        WHEN("create_and_open is called with write-only permissions") {
            auto file = File::create_and_open(path.c_str(), File::Permissions::WriteOnly);

            AND_WHEN("get_path is called") {
                const auto returnedPath = file.get_path();

                THEN("it matches the path provided") {
                    REQUIRE(returnedPath == path);
                }
            }

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

                                AND_WHEN("remove is called") {
                                    const bool removed = file.remove();

                                    THEN("it returns success") {
                                        REQUIRE(removed);
                                    }

                                    THEN("the file no longer exists") {
                                        REQUIRE(! std::filesystem::exists(path));
                                    }

                                    AND_WHEN("get_path is called") {
                                        const auto returnedPath = file.get_path();

                                        THEN("it returns an empty string") {
                                            REQUIRE(returnedPath.empty());
                                        }
                                    }
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

            AND_WHEN("get_path is called") {
                const auto returnedPath = file.get_path();

                THEN("it matches the path provided") {
                    REQUIRE(returnedPath == path);
                }
            }

            THEN("the file is open") {
                REQUIRE(check_file_open_state(path) == FileOpenState::Open);

                AND_WHEN("a file is move constructed from this file") {
                    File newFile(std::move(file));

                    AND_WHEN("get_path is called on the moved-from file") {
                        const auto returnedPath = file.get_path();

                        THEN("it returns an empty string") {
                            REQUIRE(returnedPath.empty());
                        }
                    }

                    AND_WHEN("get_path is called on the moved-to file") {
                        const auto returnedPath = newFile.get_path();

                        THEN("it matches the original path") {
                            REQUIRE(returnedPath == path);
                        }
                    }

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

        AND_WHEN("get_path is called") {
            const auto path = file.get_path();

            THEN("it returns an empty string") {
                REQUIRE(path.empty());
            }
        }
    }

    GIVEN("a non-zero 4-byte unsigned integer") {
        const uint32_t integer = 0xFF00FFAA;

        WHEN("create_and_open_temporary is called") {
            File file = File::create_and_open_temporary();

            AND_WHEN("the integer is it written to the file") {
                {
                    const auto bytesWritten = file.write(integer);

                    THEN("it reports 4 bytes were written") {
                        REQUIRE(bytesWritten == 4);
                    }
                }

                AND_WHEN("seek is called back to the start of the file") {
                    file.seek(0, File::OffsetInterpretation::Absolute);

                    AND_WHEN("the integer is read back") {
                        uint32_t readBackInteger = 0;
                        const auto bytesRead = file.read(readBackInteger);

                        THEN("4 bytes are reported to be read") {
                            REQUIRE(bytesRead == 4);
                        }

                        THEN("the read back integer matches what was written") {
                            REQUIRE(readBackInteger == integer);
                        }
                    }
                }
            }
        }
    }

    GIVEN("some chars in a c-style array") {
        const char source[] = "hi there";

        WHEN("create_and_open_temporary is called") {
            File file = File::create_and_open_temporary();

            AND_WHEN("the chars are written to the file") {
                const auto bytesWritten = file.write(source);

                THEN("the expected number of bytes is written") {
                    REQUIRE(bytesWritten == sizeof(source));
                }

                AND_WHEN("the file offset is set back to the start") {
                    file.seek(0, File::OffsetInterpretation::Absolute);

                    AND_WHEN("the data is read back") {
                        char target[sizeof(source)] = { };
                        const auto bytesRead = file.read(target);

                        THEN("the number of bytes read is correct") {
                            REQUIRE(bytesRead == sizeof(target));
                        }

                        THEN("the data matches what was written") {
                            static_assert(sizeof(source) == sizeof(target));
                            REQUIRE(memcmp(source, target, sizeof(source)) == 0);
                        }
                    }
                }
            }
        }
    }

    GIVEN("5 non-zero bytes") {
        constexpr std::array<std::byte, 5> data = {
            std::byte{1},
            std::byte{2},
            std::byte{3},
            std::byte{5},
            std::byte{8}
        };

        WHEN("create_and_open_temporary is called") {
            File file = File::create_and_open_temporary();

            AND_WHEN("get_path is called") {
                const auto path = file.get_path();

                THEN("it returns an empty string") {
                    REQUIRE(path.empty());
                }
            }

            AND_WHEN("write is called") {
                {
                    const auto bytesWritten = file.write(data);

                    THEN("5 bytes are written") {
                        REQUIRE(bytesWritten == 5);
                    }
                }

                AND_WHEN("seek is called to get back to the start of the file") {
                    {
                        const auto offset = file.seek(0, File::OffsetInterpretation::Absolute);

                        THEN("it returns 0") {
                            REQUIRE(offset == 0);
                        }
                    }

                    AND_WHEN("it is read back") {
                        {
                            std::array<std::byte, 5> readBack;
                            readBack.fill(std::byte{0});

                            const auto bytesRead = file.read(readBack);

                            THEN("5 bytes are read") {
                                REQUIRE(bytesRead == 5);
                            }

                            THEN("it matches the data written") {
                                REQUIRE(data == readBack);
                            }
                        }

                        AND_WHEN("seek is called to go back 4 bytes") {
                            {
                                const auto offset = file.seek(-4, File::OffsetInterpretation::RelativeToCurrentPosition);

                                THEN("it returns 1") {
                                    REQUIRE(offset == 1);
                                }
                            }

                            AND_WHEN("2 bytes are read back") {
                                {
                                    std::array<std::byte, 2> readBack;
                                    readBack.fill(std::byte{0});

                                    const auto bytesRead = file.read(readBack);

                                    THEN("2 bytes are read") {
                                        REQUIRE(bytesRead == 2);
                                    }

                                    THEN("they match what was written to that part of the file") {
                                        REQUIRE(readBack[0] == data[1]);
                                        REQUIRE(readBack[1] == data[2]);
                                    }
                                }

                                AND_WHEN("seek is called to go to one before the end of the file") {
                                    {
                                        const auto offset = file.seek(-1, File::OffsetInterpretation::RelativeToEndOfFile);

                                        THEN("it returns 4") {
                                            REQUIRE(offset == 4);
                                        }
                                    }

                                    AND_WHEN("the last byte of the file is read") {
                                        {
                                            auto lastByte = std::byte{0};
                                            const auto bytesRead = file.read({ &lastByte, 1 });

                                            THEN("1 byte is read") {
                                                REQUIRE(bytesRead == 1);
                                            }

                                            THEN("it matches what was written") {
                                                REQUIRE(lastByte == data.back());
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

