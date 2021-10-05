#pragma once

#include <cstddef>
#include <span>
#include <string_view>

namespace signalsafe {
    class File {
    public:
        //!
        //! \brief  Constructs an instance that refers to no file, much like std::ifstream.
        //!
        File() = default;
        virtual ~File() = default;

        // non-copyable
        File(const File&) = delete;
        File& operator=(const File&) = delete;

        // moveable
        File(File&&) = default;
        File& operator=(File&&) = default;

        //!
        //! \brief  Creates and opens a new file at the path provided.
        //!
        //! \param[in]  path  The path of the file to be created.
        //!
        //! \returns  The created file, opened.
        //!
        static File create_and_open(std::string_view path);

        //!
        //! \brief  Opens an existing file at the path provided.
        //!
        //! \param[in]  path  The path to the file to be opened (must be null terminated).
        //!
        //! \returns  The opened file.
        //!
        static File open_existing(std::string_view path);

        //!$
        //! \brief  Reads the requested number of bytes into the target.
        //!
        //! \param[out]  target  Where to write the read bytes.
        //!
        //! \returns  the number of bytes read.
        //!
        std::size_t read(std::span<std::byte> target);

    private:
        using file_descriptor = int;

        file_descriptor m_fileDescriptor = -1;
    };
}
