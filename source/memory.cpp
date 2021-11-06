#include <signalsafe/memory.hpp>

#include <cstring>

std::size_t signalsafe::memory::copy_no_overlap(std::span<const std::byte> source, std::span<std::byte> target) {
    const auto n = std::min(source.size_bytes(), target.size_bytes());
    memcpy(target.data(), source.data(), n);
    return n;
}

std::size_t signalsafe::memory::copy_with_overlap(std::span<const std::byte> source, std::span<std::byte> target) {
    const auto n = std::min(source.size_bytes(), target.size_bytes());
    memmove(target.data(), source.data(), n);
    return n;
}

