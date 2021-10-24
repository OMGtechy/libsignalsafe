#pragma once

#include <climits>
#include <concepts>
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

        using file_descriptor_t = int;

        enum class OffsetInterpretation : decltype(SEEK_SET) {
            Absolute = SEEK_SET,
            RelativeToCurrentPosition = SEEK_CUR,
            RelativeToEndOfFile = SEEK_END
        };

        enum class Permissions : decltype(O_RDWR) {
            ReadOnly = O_RDONLY,
            WriteOnly = O_WRONLY,
            ReadWrite = O_RDWR
        };

        enum class DestroyAction {
            Nothing,
            Close
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
        //! \brief  Creates and opens a new temporary file.
        //!
        //! \returns  The created file, opened.
        //!
        static File create_and_open_temporary();

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
        //! \brief  Creates a File instance from an existing file descriptor.
        //!
        //! \param[in]  fd  The file desriptor to associated the instance with.
        //!
        //! \returns  A file associated with the provided file descriptor.
        //!
        static File from_file_descriptor(file_descriptor_t fd);

        //!
        //! \brief  Reads the requested bytes into the target.
        //!
        //! \param[out]  target  Where to write the read bytes.
        //!
        //! \returns  The number of bytes read.
        //!
        std::size_t read(std::span<std::byte> target);
        std::size_t read(std::span<char> target);

        //!
        //! \brief  Reads sizeof(T) bytes into the target.
        //!
        //! \tparam  T  The type of the target.
        //!
        //! \param[in]  target  Where to write the read bytes.
        //!
        //! \returns  The number of bytes read.
        //!
        template <typename T>
        std::size_t read(T& target) requires std::integral<T> {
            return read(std::span<std::byte, sizeof(T)>(reinterpret_cast<std::byte*>(&target), sizeof(T)));
        }

        //!
        //! \brief  Writes the provided bytes to the file.
        //!
        //! \param[in]  source  Where to read the bytes from.
        //!
        //! \returns  The number of bytes written.
        //!
        std::size_t write(std::span<const std::byte> source);
        std::size_t write(std::span<const char> source);

        //!
        //! \brief  Writes sizeof(T) bytes to the target.
        //!
        //! \tparam  T  The type of the source.
        //!
        //! \param[in]  Where to read the bytes from.
        //!
        //! \returns  The number of bytes written.
        //!
        template <typename T>
        std::size_t write(const T& source) requires std::integral<T> {
            return write(std::span<const std::byte, sizeof(T)>(reinterpret_cast<const std::byte*>(&source), sizeof(T)));
        }

        //!
        //! \brief  Closes the file.
        //!
        //! \returns  True if successful, false otherwise.
        //!
        bool close();

        //!
        //! \brief  Removes (a.k.a. deletes) the file.
        //!
        //! \returns  True if successful, false otherwise.
        //!
        bool remove();

        //!
        //! \brief  Changes the read/write file offset.
        //!
        //! \param[in]  offset                The offset to set the file to.
        //! \param[in]  offsetInterpretation  How to intepret the offset.
        //!
        //! \returns  The new offset, or -1 if an error occured.
        //!
        off_t seek(off_t offset, OffsetInterpretation offsetInterpretation);

        //!
        //! \brief  Gets the internal file descriptor.
        //!
        //! \returns  The internal file descriptor, or -1 if there isn't one.
        //!
        file_descriptor_t get_file_descriptor() const;

        //!
        //! \brief  Gets the path associated with the open file.
        //!
        //! \returns  The open file path, or an empty string if there isn't one.
        //!
        //! \note  A temporary file may not have a path associated with it.
        //!
        std::string_view get_path() const;

        //!
        //! \brief  Gets the destroy action.
        //!
        //! \returns  The destroy action.
        //!
        DestroyAction get_destroy_action() const;

        //!
        //! \brief  Sets the destroy action.
        //!
        //! \param[in]  destroyAction  The new destroy action.
        //!
        void set_destroy_action(DestroyAction destroyAction);

    protected:
        void create_and_open_internal(std::string_view path, Permissions permissions);
        void create_and_open_temporary_internal();
        void open_existing_internal(std::string_view path, Permissions permissions);
        void from_file_descriptor_internal(file_descriptor_t fd);
 
    private:
        std::array<char, PATH_MAX + 1 /* null terminator */> m_path = { };
        file_descriptor_t m_fileDescriptor = -1;
        DestroyAction m_destroyAction = DestroyAction::Close;
    };
}
