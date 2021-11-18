#include "signalsafe/time.hpp"

#include <cassert>

using signalsafe::time::TimeSpecification;

TimeSpecification signalsafe::time::now(const clockid_t clockID) {
    timespec timespecNow { 0, 0 };
    [[maybe_unused]] const int returnValue = clock_gettime(clockID, &timespecNow);

    // As far as I know, the only reason it can fail are:
    //
    // 1) The timespec is a bad pointer (clearly not the case here)
    // 2) The clock type is not supported.
    //
    // The latter is the only plausible one, and that seems reasonable to assert against.
    assert(returnValue == 0);

    return { timespecNow.tv_sec, timespecNow.tv_nsec };
}

