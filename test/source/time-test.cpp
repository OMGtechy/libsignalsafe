#include "signalsafe-test.hpp"
#include <signalsafe/time.hpp>

using signalsafe::time::now;

SCENARIO("signalsafe::time") {
    GIVEN("a monotonic clock") {
        WHEN("now(...) is called") {
            const auto time = now(CLOCK_MONOTONIC);

            THEN("the returned time is non-zero") {
                // Apologies to the time travels using this library
                REQUIRE(time.seconds != 0);
                REQUIRE(time.nanoseconds != 0);
            }
        }
    }

    GIVEN("a realtime clock") {
        WHEN("now(...) is called") {
            const auto time = now(CLOCK_REALTIME);

            THEN("the returned time is non-zero") {
                REQUIRE(time.seconds != 0);
                REQUIRE(time.nanoseconds != 0);
            }
        }
    }

    GIVEN("a process cputime clock") {
        WHEN("now(...) is called") {
            const auto time = now(CLOCK_PROCESS_CPUTIME_ID);

            THEN("the returned time is non-zero") {
                // It seems reasonable to get an answer of 0 seconds here,
                // so that test has been missed.
                //
                // If we manage to run in 0 nanoseconds,
                // the library is too fast and should be nerfed
                REQUIRE(time.nanoseconds != 0);
            }
        }
    }

    GIVEN("a thread cputime clock") {
        WHEN("now(...) is called") {
            const auto time = now(CLOCK_THREAD_CPUTIME_ID);

            THEN("the returned time is non-zero") {
                // It seems reasonable to get an answer of 0 seconds here,
                // so that test has been missed.
                REQUIRE(time.nanoseconds != 0);
            }
        }
    }
}

