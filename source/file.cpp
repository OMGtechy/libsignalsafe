#include "signalsafe/file.hpp"

#include <cassert>
#include <cerrno>

#include <fcntl.h>
#include <unistd.h>

using signalsafe::File;

File::~File() {
    if(m_fileDescriptor != -1) {
        const auto closeSuccess = this->close();
        assert(closeSuccess);
    }
}

File::File(File&& other) {
    *this = std::move(other);
}

File& File::operator=(File&& other) {
    if(m_fileDescriptor != -1) {
        const auto closeSuccess = this->close();
        assert(closeSuccess);
    }

    this->m_fileDescriptor = other.m_fileDescriptor;
    other.m_fileDescriptor = -1;

    return *this;
}

File File::create_and_open(std::string_view path) {
    File file;

    file.m_fileDescriptor = ::open(path.data(), O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
    assert(file.m_fileDescriptor != -1);

    return file;
}

File File::open_existing(std::string_view path) {
    File file;

    file.m_fileDescriptor = ::open(path.data(), O_RDONLY);
    assert(file.m_fileDescriptor != -1);

    return file;
}

std::size_t File::read(std::span<std::byte> target) {
    assert(m_fileDescriptor != -1);

    std::size_t bytesRead = 0;

    while(target.size() > 0) {
        const auto newBytesReadOrError = ::read(
            m_fileDescriptor,
            target.data(),
            target.size()
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

std::size_t File::write(std::span<const std::byte> source) {
    std::size_t bytesWritten = 0;

    while(source.size() > 0) {
        const auto newBytesWrittenOrError = ::write(
            m_fileDescriptor,
            source.data(),
            source.size()
        );

        if (newBytesWrittenOrError < 0) {
            // Just in case any subsequent calls modify it.
            const auto errorCode = errno;

            // This is the only "acceptable" error;
            // it can happen when a signal fires mid-write.
            assert(errorCode == EINTR);

            continue;
        }

        const auto newBytesWritten = static_cast<std::size_t>(newBytesWrittenOrError);
        source = decltype(source)(source.data() + newBytesWritten, source.size() - newBytesWritten);
        bytesWritten += static_cast<size_t>(newBytesWritten);
    }

    return bytesWritten;
}

bool File::close() {
    if(m_fileDescriptor == -1) {
        return false;
    }

    do {
        switch(::close(m_fileDescriptor)) {
        case EINTR: continue;
        case 0: {
            m_fileDescriptor = -1;
            return true;
        }
        default: {
            assert(false);
            return false;
        }}
    } while(true);
}