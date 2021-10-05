#include "signalsafe/file.hpp"

#include <cassert>
#include <cerrno>

#include <fcntl.h>
#include <unistd.h>

using signalsafe::File;

File File::create_and_open(std::string_view path) {
    File file;

    file.m_fileDescriptor = ::open(path.data(), O_RDWR | O_CREAT | O_EXCL);

    return file;
}

File File::open_existing(std::string_view path) {
    File file;

    file.m_fileDescriptor = ::open(path.data(), O_RDONLY);

    return file;
}

std::size_t File::read(std::span<std::byte> target) {
    assert(m_fileDescriptor != -1);

    std::size_t bytesRead = 0;

    while(target.size() > 0) {
        const auto newBytesReadOrError = ::read(
            m_fileDescriptor,
            target.data(),
            static_cast<ssize_t>(target.size())
        );

        if (newBytesReadOrError < 0) {
            // Just in case any subsequent calls modify it.
            const auto errorCode = errno;

            // This is the only "acceptable" error;
            // it can happen when a signal fires mid-read.
            assert(errorCode == EINTR);

            continue;
        }

        if (newBytesReadOrError == 0) {
            // End of file.
            return bytesRead;
        }

        const auto newBytesRead = static_cast<std::size_t>(newBytesReadOrError);
        target = decltype(target)(target.data() + newBytesRead, target.size() - newBytesRead);
        bytesRead += newBytesRead;
    }

    return bytesRead;
}

