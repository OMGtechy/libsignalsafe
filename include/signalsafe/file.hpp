#pragma once

#include <cstddef>
#include <filesystem>
#include <span>
#include <string_view>

#include <fcntl.h>

namespace signalsafe {
    class File {
    public:
        //!
        //! \brief  Constructs an instance that refers to no file, much like std::ifstream.
        //!
        File() = default;
        virtual ~File();

        // non-copyable
        File(const File&) = delete;
        File& operator=(const File&) = delete;

        // moveable
        File(File&&);
        File& operator=(File&&);

        using file_descriptor = int;

        enum class Permissions : decltype(O_RDWR) {
            ReadOnly = O_RDONLY,
            WriteOnly = O_WRONLY,
            ReadWrite = O_RDWR
        };

        //!
        //! \brief  Creates and opens a new file at the path provided.
        //!
        //! \param[in]  path         The path of the file to be created.
        //! \param[in]  permissions  The permissions to create the file with.
        //!
        //! \returns  The created file, opened.
        //!
        static File create_and_open(std::string_view path, Permissions permissions);

        //!
        //! \brief  Opens an existing file at the path provided.
        //!
        //! \param[in]  path         The path to the file to be opened (must be null terminated).
        //! \param[in]  permissions  The permissions to create the file with.
        //!
        //! \returns  The opened file.
        //!
        static File open_existing(std::string_view path, Permissions permissions);

        //!
        //! \brief  Reads the requested bytes into the target.
        //!
        //! \param[out]  target  Where to write the read bytes.
        //!
        //! \returns  The number of bytes read.
        //!
        std::size_t read(std::span<std::byte> target);

        //!
        //! \brief  Writes the requested bytes to the target.
        //!
        //! \param[in]  source  Where to read the bytes from.
        //!
        //! \returns  The number of bytes written.
        //!
        std::size_t write(std::span<const std::byte> source);

        //!
        //! \brief  Closes the file.
        //!
        //! \returns  True if successful, false otherwise.
        //!
        bool close();

        //!
        //! \brief  Gets the internal file descriptor.
        //!
        //! \returns  The internal file descriptor, or -1 if there isn't one.
        //!
        file_descriptor get_file_descriptor() const;

    private:

        file_descriptor m_fileDescriptor = -1;
    };
}
