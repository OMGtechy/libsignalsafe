#include "signalsafe-test.hpp"
#include <signalsafe/string.hpp>

using signalsafe::string::format;

SCENARIO("signalsafe::string") {
    GIVEN("a format string with no format specifiers") {
        const char formatStr[] = "testing";

        WHEN("it is formatted") {
            char targetStr[sizeof(formatStr)] = { };
            const auto bytesWritten = format(formatStr, targetStr);

            THEN("the expected number of bytes are written") {
                REQUIRE(bytesWritten == sizeof(targetStr));
            }
 
            THEN("it matches the original format string") {
                REQUIRE(std::string(formatStr) == std::string(targetStr));
            }
        }
    }

    GIVEN("a format string with a single format specifier") {
        const char formatStr[] = "format: %";

        WHEN("it is formatted with an int32_t") {
            const char expectedStr[] = "format: 12341234";
            char targetStr[sizeof(expectedStr)] = { };

            const auto bytesWritten = format(formatStr, targetStr, int32_t{12341234});

            THEN("the expected number of bytes are written") {
                REQUIRE(bytesWritten == sizeof(targetStr));
            }

            THEN("it matches the expected result") {
                REQUIRE(std::string(targetStr) == std::string(expectedStr));
            }
        }

        WHEN("it is formatted with an uint32_t") {
            const char expectedStr[] = "format: 22341234";
            char targetStr[sizeof(expectedStr)] = { };

            const auto bytesWritten = format(formatStr, targetStr, uint32_t{22341234});

            THEN("the expected number of bytes are written") {
                REQUIRE(bytesWritten == sizeof(targetStr));
            }

            THEN("it matches the expected result") {
                REQUIRE(std::string(targetStr) == std::string(expectedStr));
            }
        }

        WHEN("it is formatted with an int64_t") {
            const char expectedStr[] = "format: 4611686018427387904";
            char targetStr[sizeof(expectedStr)] = { };

            const auto bytesWritten = format(formatStr, targetStr, int64_t{4611686018427387904});

            THEN("the expected number of bytes are written") {
                REQUIRE(bytesWritten == sizeof(targetStr));
            }

            THEN("it matches the expected result") {
                REQUIRE(std::string(targetStr) == std::string(expectedStr));
            }
        }

        WHEN("it is formatted with a uint64_t") {
            const char expectedStr[] = "format: 8611686018427387904";
            char targetStr[sizeof(expectedStr)] = { };

            const auto bytesWritten = format(formatStr, targetStr, int64_t{8611686018427387904});

            THEN("the expected number of bytes are written") {
                REQUIRE(bytesWritten == sizeof(targetStr));
            }

            THEN("it matches the expected result") {
                REQUIRE(std::string(targetStr) == std::string(expectedStr));
            }
        }

        WHEN("it is formatted with a literal 0") {
            const char expectedStr[] = "format: 0";
            char targetStr[sizeof(expectedStr)] = { };

            const auto bytesWritten = format(formatStr, targetStr, 0);

            THEN("the expected number of bytes are written") {
                REQUIRE(bytesWritten == sizeof(targetStr));
            }

            THEN("it matches the expected result") {
                REQUIRE(std::string(targetStr) == std::string(expectedStr));
            }
        }
    }
}
