#include "signalsafe/string.hpp"

#include <cassert>
#include <cmath>
#include <cstring>

#include <signalsafe/memory.hpp>

using signalsafe::memory::copy_no_overlap;

// An unnamed namespace won't do the trick since this is technically a header file.
namespace signalsafe::string::impl {
    template <typename T>
    std::size_t stringify(std::span<char> targetStr, T value) requires std::is_same_v<T, const char*>
                                                                    || std::is_same_v<T,       char*> {
        return copy_no_overlap(std::span<const char>{ value, strlen(value) }, targetStr);
    }

    template <typename T>
    std::size_t stringify(std::span<char> targetStr, T value) requires std::is_same_v<T, uint32_t>
                                                                    || std::is_same_v<T, uint64_t> {
        assert(targetStr.size() > 0);

        if (value == 0) {
            targetStr[0] = '0';
            return 1;
        }

        const auto digitsInValue = static_cast<std::size_t>(std::floor(std::log10(value))) + 1;
        auto digitsLeft = digitsInValue;

        auto currentPowerOf10 = static_cast<std::size_t>(std::floor(std::pow(10, digitsLeft - 1)));

        std::size_t bytesWritten = 0;

        do {
            const auto digitToWrite = (value / currentPowerOf10);
            targetStr[0] = '0' + digitToWrite;
            value -= digitToWrite * currentPowerOf10;
            targetStr = targetStr.last(targetStr.size() - 1);
            bytesWritten += 1;
            digitsLeft -= 1;
            currentPowerOf10 /= 10;
        } while(targetStr.size() != 0 && digitsLeft != 0);

        return bytesWritten;
    }

    template <typename T>
    std::size_t stringify(std::span<char> targetStr, T value) requires std::is_same_v<T, int32_t>
                                                                    || std::is_same_v<T, int64_t> {
        assert(targetStr.size() > 0);

        std::size_t bytesWritten = 0;
        if (value < 0) {
            value = value * -1;
            targetStr[0] = '-';
            targetStr = targetStr.last(targetStr.size() - 1);
            bytesWritten = 1;
        }

        return bytesWritten + stringify(targetStr, std::make_unsigned_t<T>(value));
    }
}

namespace signalsafe::string {
    std::size_t format(
        std::span<const char> formatStr,
        std::span<char> targetStr) {

        return copy_no_overlap(formatStr, targetStr);
    }

    template <typename T, typename... ArgTypes>
    std::size_t format(
        std::span<const char> formatStr,
        std::span<char> targetStr,
        const T& firstArg,
        const ArgTypes... remainingArgs) {

        const size_t bytesToProcess = std::min(formatStr.size(), targetStr.size());

        // The return value is either:
        // 1) NULL, meaning the we're done.
        // 2) A pointer to the character after the % in the targetStr buffer,
        //    meaning some formatting needs doing.
        const char* const formatCharacterTarget = static_cast<char*>(
            memccpy(
                targetStr.data(),
                formatStr.data(),
                '%',
                bytesToProcess
            )
        );

        if (formatCharacterTarget == nullptr) {
            return bytesToProcess;
        }

        assert(*(formatCharacterTarget - 1) == '%');

        auto bytesWritten = static_cast<std::size_t>(formatCharacterTarget - targetStr.data()) - 1 /* for the % */;
        formatStr = formatStr.last(formatStr.size() - (bytesWritten + 1));
        targetStr = targetStr.last(targetStr.size() - bytesWritten);

        {
            const auto newBytesWritten = impl::stringify(targetStr, firstArg);
            targetStr = targetStr.last(targetStr.size() - newBytesWritten);
            bytesWritten += newBytesWritten;
        }

        return bytesWritten + format(formatStr, targetStr, remainingArgs...);
    }
}
