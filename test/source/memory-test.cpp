#include "signalsafe-test.hpp"
#include <signalsafe/memory.hpp>

#include <array>

using signalsafe::memory::copy_no_overlap;
using signalsafe::memory::copy_with_overlap;

SCENARIO("signalsafe::memory") {
    GIVEN("some chars") {
        const std::array<char, 9> data = {
            'a', 'b', 'c',
            1, 2, 3,
            0, 42, 0
        };

        WHEN("they are copied to a std::array that doesn't overlap, with enough room for all of them, using copy_no_overlap") {
            std::array<char, data.size()> target;
            target.fill('d');

            const auto bytesCopied = copy_no_overlap(data, target);

            THEN("it reports the expected number have been copied") {
                REQUIRE(bytesCopied == data.size());
            }

            THEN("the target contains the expected data") {
                REQUIRE(data == target);
            }
        }

        WHEN("they are copied to a std::array, with enough room for all of them, using copy_with_overlap") {
            std::array<char, data.size()> target;
            target.fill('d');

            const auto bytesCopied = copy_with_overlap(data, target);

            THEN("it reports the expected number have been copied") {
                REQUIRE(bytesCopied == data.size());
            }

            THEN("the target contains the expected data") {
                REQUIRE(data == target);
            }
        }

        WHEN("they are copied to a c-array that doesn't have enough room, using copy_no_overlap") {
            char target[6] = { };

            const auto bytesCopied = copy_no_overlap(data, target);

            THEN("it reports the expected number have been copied") {
                REQUIRE(bytesCopied == sizeof(target));
            }

            THEN("the target contains the expected data") {
                REQUIRE(target[0] == data[0]);
                REQUIRE(target[1] == data[1]);
                REQUIRE(target[2] == data[2]);
                REQUIRE(target[3] == data[3]);
                REQUIRE(target[4] == data[4]);
                REQUIRE(target[5] == data[5]);
            }
        }

        WHEN("they are copied to a c-array that doesn't have enough room, using copy_with_overlap") {
            char target[6] = { };

            const auto bytesCopied = copy_with_overlap(data, target);

            THEN("it reports the expected number have been copied") {
                REQUIRE(bytesCopied == sizeof(target));
            }

            THEN("the target contains the expected data") {
                REQUIRE(target[0] == data[0]);
                REQUIRE(target[1] == data[1]);
                REQUIRE(target[2] == data[2]);
                REQUIRE(target[3] == data[3]);
                REQUIRE(target[4] == data[4]);
                REQUIRE(target[5] == data[5]);
            }
        }

        WHEN("they are copied to a c-array that has more than enough room, using copy_no_overlap") {
            char target[11] = { };

            const auto bytesCopied = copy_no_overlap(data, target);

            THEN("it reports the expected number have been copied") {
                REQUIRE(bytesCopied == data.size());
            }

            THEN("the target contains the expected data") {
                REQUIRE(target[0] == data[0]);
                REQUIRE(target[1] == data[1]);
                REQUIRE(target[2] == data[2]);
                REQUIRE(target[3] == data[3]);
                REQUIRE(target[4] == data[4]);
                REQUIRE(target[5] == data[5]);
                REQUIRE(target[6] == data[6]);
                REQUIRE(target[7] == data[7]);
                REQUIRE(target[8] == data[8]);
            }

            THEN("the extra bytes are unchanged") {
                REQUIRE(target[9]  == 0);
                REQUIRE(target[10] == 0);
            }
        }

        WHEN("they are copied to a c-array that has more than enough room, using copy_with_overlap") {
            char target[11] = { };

            const auto bytesCopied = copy_with_overlap(data, target);

            THEN("it reports the expected number have been copied") {
                REQUIRE(bytesCopied == data.size());
            }

            THEN("the target contains the expected data") {
                REQUIRE(target[0] == data[0]);
                REQUIRE(target[1] == data[1]);
                REQUIRE(target[2] == data[2]);
                REQUIRE(target[3] == data[3]);
                REQUIRE(target[4] == data[4]);
                REQUIRE(target[5] == data[5]);
                REQUIRE(target[6] == data[6]);
                REQUIRE(target[7] == data[7]);
                REQUIRE(target[8] == data[8]);
            }

            THEN("the extra bytes are unchanged") {
                REQUIRE(target[9]  == 0);
                REQUIRE(target[10] == 0);
            }
        }
    }

    GIVEN("some data") {
        std::array<char, 6> data = {
            3, 1, 2,
            5, 4, 9
        };

        WHEN("copy_with_overlap is called with overlapping memory regions") {
            auto target = std::span<char>{ &data[1], data.size() - 1 };
            const auto bytesCopied = copy_with_overlap(data, target);

            THEN("it reports the expected number of bytes have been copied") {
                REQUIRE(bytesCopied == 5);
            }

            THEN("the result bytes are as expected") {
                REQUIRE(data == std::array<char, 6>{3, 3, 1, 2, 5, 4});
            }
        }
    }

    GIVEN("some data") {
        const std::array<char, 2> data = { 1, 2 };

        {
            std::array<char, 2> target;
            target.fill('a');

            WHEN("copy_no_overlap is called with them manually constructed rvalue span<char>s") {
                const auto bytesCopied = copy_no_overlap(
                    std::span<const char>(data.data(), data.size()),
                    std::span<char>(target.data(), target.size())
                );

                THEN("the number of bytes copied is as expected") {
                    REQUIRE(bytesCopied == data.size());
                }

                THEN("the data copied is correct") {
                    REQUIRE(data == target);
                }
            }
        }

        {

            std::array<char, 2> target;
            target.fill('a');

            WHEN("copy_with_overlap is called with them with manually constructed rvalue span<char>s") {
                const auto bytesCopied = copy_with_overlap(
                    std::span<const char>(data.data(), data.size()),
                    std::span<char>(target.data(), target.size())
                );

                THEN("the number of bytes copied is as expected") {
                    REQUIRE(bytesCopied == data.size());
                }

                THEN("the data copied is correct") {
                    REQUIRE(data == target);
                }
            }
        }
    }
}

