#pragma once

#include <cstddef>
#include <iterator>
#include <span>

namespace signalsafe::memory {
    //!
    //! \brief  Copies memory, where the source and target do not overlap.
    //!
    //! \param[in]   source  Where to read from.
    //! \param[out]  target  Where to write to.
    //!
    //! \returns  The number of bytes copied.
    //!
    std::size_t copy_no_overlap(std::span<const std::byte> source, std::span<std::byte> target);

    template <typename LhsT, typename RhsT>
    std::size_t copy_no_overlap(const LhsT& source, RhsT& target) {
        using lhs_element_t = decltype(source[0]);
        using rhs_element_t = decltype(target[0]);

        if constexpr (std::is_same_v<LhsT, std::span<lhs_element_t>>
                   && ! std::is_same_v<lhs_element_t, std::byte>) {
            return copy_no_overlap(source.as_bytes(), target);
        }

        if constexpr (std::is_same_v<RhsT, std::span<rhs_element_t>>
                   && ! std::is_same_v<rhs_element_t, std::byte>) {
            return copy_no_overlap(source, target.as_wriable_bytes());
        }

        static_assert(std::contiguous_iterator<decltype(std::begin(source))>);
        static_assert(std::contiguous_iterator<decltype(std::begin(target))>);

        return copy_no_overlap(
            std::span<const std::byte>( reinterpret_cast<const std::byte*>(&source[0]), std::size(source) * sizeof(lhs_element_t) ),
            std::span<      std::byte>( reinterpret_cast<      std::byte*>(&target[0]), std::size(target) * sizeof(rhs_element_t) )
        );
    }

    //!
    //! \brief  Copies memory, where the source and target may overlap.
    //!
    //! \param[in]   source  Where to read from.
    //! \param[out]  target  Where to write to.
    //!
    //! \returns  The number of bytes copied.
    //!
    std::size_t copy_with_overlap(std::span<const std::byte> source, std::span<std::byte> target);

    template <typename LhsT, typename RhsT>
    std::size_t copy_with_overlap(const LhsT& source, RhsT& target) {
        using lhs_element_t = decltype(source[0]);
        using rhs_element_t = decltype(target[0]);

        static_assert(std::contiguous_iterator<decltype(std::begin(source))>);
        static_assert(std::contiguous_iterator<decltype(std::begin(target))>);

        return copy_with_overlap(
            std::span<const std::byte>( reinterpret_cast<const std::byte*>(&source[0]), std::size(source) * sizeof(lhs_element_t) ),
            std::span<      std::byte>( reinterpret_cast<      std::byte*>(&target[0]), std::size(target) * sizeof(rhs_element_t) )
        );
    }
}

