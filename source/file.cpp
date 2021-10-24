#include "signalsafe/file.hpp"

#include <cassert>
#include <cerrno>
#include <cstring>
#include <filesystem>

#include <fcntl.h>
#include <unistd.h>

using signalsafe::File;

namespace {
    void destroy(File& file) {
        switch(file.get_destroy_action()) {
        case File::DestroyAction::Nothing: return;
        case File::DestroyAction::Close: {
            if (file.get_file_descriptor() != -1) {
                const auto closeSuccess = file.close();
                assert(closeSuccess);
            }
        }}
    }
}

File::~File() {
    destroy(*this);
}

File::File(File&& other) {
    *this = std::move(other);
}

File& File::operator=(File&& other) {
    destroy(*this);

    this->m_fileDescriptor = other.m_fileDescriptor;
    other.m_fileDescriptor = -1;

    this->m_path = other.m_path;
    other.m_path.fill('\0');

    this->m_destroyAction = other.m_destroyAction;
    other.m_destroyAction = DestroyAction::Close;

    return *this;
}

File File::create_and_open(std::string_view path, Permissions permissions) {
    File file;
    file.create_and_open_internal(path, permissions);
    return file;
}

void File::create_and_open_internal(std::string_view path, Permissions permissions) {
    m_fileDescriptor = ::open(
        path.data(),
        static_cast<std::underlying_type_t<decltype(permissions)>>(permissions) | O_CREAT | O_EXCL,
        S_IRUSR | S_IWUSR
    );

    assert(m_fileDescriptor != -1);

    ::strncpy(m_path.data(), path.data(), std::min(m_path.size(), path.size()));
}

File File::create_and_open_temporary() {
    File file;
    file.create_and_open_temporary_internal();
    return file;
}

void File::create_and_open_temporary_internal() {
    m_fileDescriptor = ::open(
        "/tmp",
        O_TMPFILE | O_RDWR,
        S_IRUSR | S_IWUSR
    );

    assert(m_fileDescriptor != -1);
}

File File::open_existing(std::string_view path, Permissions permissions) {
    File file;
    file.open_existing_internal(path, permissions);
    return file;
}

void File::open_existing_internal(std::string_view path, Permissions permissions) {
    m_fileDescriptor = ::open(
        path.data(),
        static_cast<std::underlying_type_t<decltype(permissions)>>(permissions)
    );

    assert(m_fileDescriptor != -1);

    ::strncpy(m_path.data(), path.data(), std::min(m_path.size(), path.size()));
}

File File::from_file_descriptor(const file_descriptor_t fd) {
    File file;
    file.from_file_descriptor_internal(fd);
    return file;
}

void File::from_file_descriptor_internal(const file_descriptor_t fd) {
    m_fileDescriptor = fd;
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
        target = target.last(target.size() - newBytesRead);
        bytesRead += newBytesRead;
    }

    return bytesRead;
}

std::size_t File::read(std::span<char> target) {
    return read(std::span<std::byte>(reinterpret_cast<std::byte*>(target.data()), target.size()));
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
        source = source.last(source.size() - newBytesWritten);
        bytesWritten += static_cast<size_t>(newBytesWritten);
    }

    return bytesWritten;
}

std::size_t File::write(std::span<const char> source) {
    return write(std::span<const std::byte>(reinterpret_cast<const std::byte*>(source.data()), source.size()));
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

bool File::remove() {
    if(m_fileDescriptor == -1 || m_path[0] == '\0') {
        return false;
    }

    const auto result = ::unlink(m_path.data()) == 0;

    if(result) {
        m_path.fill('\0');
    }

    return result;
}

off_t File::seek(const off_t offset, const OffsetInterpretation offsetInterpretation) {
    return ::lseek(m_fileDescriptor, offset, static_cast<std::underlying_type_t<OffsetInterpretation>>(offsetInterpretation));
}

File::file_descriptor_t File::get_file_descriptor() const {
    return m_fileDescriptor;
}

std::string_view File::get_path() const {
    return { m_path.data(), strnlen(m_path.data(), m_path.size() - 1 /* null terminator */)};
}

File::DestroyAction File::get_destroy_action() const {
    return m_destroyAction;
}

void File::set_destroy_action(const DestroyAction destroyAction) {
    m_destroyAction = destroyAction;
}

