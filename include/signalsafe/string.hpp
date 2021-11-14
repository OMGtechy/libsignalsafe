#pragma once

#include <cstdint>
#include <span>

namespace signalsafe::string {
    template <typename... ArgTypes>
    inline std::size_t format(
        std::span<const char> format,
        std::span<char> target,
        const ArgTypes... args);
}

#include "string.impl.hpp"

