#pragma once

#include <cstdint>
#include <span>

namespace signalsafe::string {
    //!
    //! \brief  Formats a string, where % delimits where to place each provided argument.
    //!
    //! \param[in]    The format string.
    //! \param[out]   Where to write the formatted string.
    //! \paran[args]  The args the use when formatting.
    //!
    //! \returns  The number of bytes written.
    //!
    template <typename... ArgTypes>
    inline std::size_t format(
        std::span<const char> format,
        std::span<char> target,
        const ArgTypes... args);
}

#include "string.impl.hpp"

