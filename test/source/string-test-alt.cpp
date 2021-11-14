#include "signalsafe-test.hpp"
#include <signalsafe/string.hpp>

using signalsafe::string::format;

// This file exists to make sure multiple translation units can use format.

SCENARIO("signalsafe::string (alt)") {
    GIVEN("a format string with a single format specifier") {
        const char formatStr[] = "format: %";

        WHEN("it is formatted with a literal non-zero") {
            const char expectedStr[] = "format: 42";
            char targetStr[sizeof(expectedStr)] = { };

            const auto bytesWritten = format(formatStr, targetStr, 42);

            THEN("the expected number of bytes are written") {
                REQUIRE(bytesWritten == sizeof(targetStr));
            }

            THEN("it matches the expected result") {
                REQUIRE(std::string(targetStr) == std::string(expectedStr));
            }
        }
    }
}
