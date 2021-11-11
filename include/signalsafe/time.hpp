#pragma once

#include <ctime>
#include <cstdint>
#include <type_traits>

namespace signalsafe::time {
    // This is needed because we assume that time_t and long are aliases for int64_t.
    // TimeSpecification may break if that's not the case.
    static_assert(std::is_same_v<int64_t, long>);
    static_assert(std::is_same_v<int64_t, time_t>);

    //!
    //! \brief  Represents a moment in time.
    //!
    //! \note   This type exists because the standard timespec might not be the same across platforms,
    //!         meaning that one reading and writing it from a file might run into issues if the size changes.
    //!
    //!         Having this means that we know exactly how big the timespec is and can read it everywhere.
    //!
    struct TimeSpecification final {
        int64_t seconds = 0;
        int64_t nanoseconds = 0;
    };

    //!
    //! \brief  A more cross-platform version of clock_gettime.
    //!
    //! \param[in]  clockID  The kind of clock you want to get the time from.
    //!
    //! \returns  The time.
    //!
    TimeSpecification now(clockid_t clockID);
}
