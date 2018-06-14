#ifndef OSMIUM_IO_DETAIL_READ_WRITE_HPP
#define OSMIUM_IO_DETAIL_READ_WRITE_HPP

/*

This file is part of Osmium (https://osmcode.org/libosmium).

Copyright 2013-2018 Jochen Topf <jochen@topf.org> and others (see README).

Boost Software License - Version 1.0 - August 17th, 2003

Permission is hereby granted, free of charge, to any person or organization
obtaining a copy of the software and accompanying documentation covered by
this license (the "Software") to use, reproduce, display, distribute,
execute, and transmit the Software, and to prepare derivative works of the
Software, and to permit third-parties to whom the Software is furnished to
do so, all subject to the following:

The copyright notices in the Software and this entire statement, including
the above license grant, this restriction and the following disclaimer,
must be included in all copies of the Software, in whole or in part, and
all derivative works of the Software, unless such copies or derivative
works are solely in the form of machine-executable object code generated by
a source language processor.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.

*/

#include <osmium/io/writer_options.hpp>

#include <cerrno>
#include <cstddef>
#include <fcntl.h>
#include <string>
#include <system_error>

#ifndef _MSC_VER
# include <unistd.h>
#else
# include <io.h>
#endif

namespace osmium {

    namespace io {

        /**
         * @brief Namespace for Osmium internal use
         */
        namespace detail {

            /**
             * Open file for writing. If the file exists, it is truncated, if
             * not, it is created. If the file name is empty or "-", no file
             * is opened and the stdout file descriptor (1) is returned.
             *
             * @param filename Name of file to be opened.
             * @param allow_overwrite If the file exists, should it be overwritten?
             * @returns File descriptor of open file.
             * @throws system_error if the file can't be opened.
             */
            inline int open_for_writing(const std::string& filename, osmium::io::overwrite allow_overwrite = osmium::io::overwrite::no) {
                if (filename.empty() || filename == "-") {
#ifdef _WIN32
                    _setmode(1, _O_BINARY);
#endif
                    return 1; // stdout
                }

                int flags = O_WRONLY | O_CREAT; // NOLINT(hicpp-signed-bitwise)
                if (allow_overwrite == osmium::io::overwrite::allow) {
                    flags |= O_TRUNC;
                } else {
                    flags |= O_EXCL;
                }
#ifdef _WIN32
                flags |= O_BINARY;
#endif
                const int fd = ::open(filename.c_str(), flags, 0666);
                if (fd < 0) {
                    throw std::system_error{errno, std::system_category(), std::string("Open failed for '") + filename + "'"};
                }
                return fd;
            }

            /**
             * Open file for reading. If the file name is empty or "-", no file
             * is opened and the stdin file descriptor (0) is returned.
             *
             * @param filename Name of file to be opened.
             * @returns File descriptor of open file.
             * @throws system_error if the file can't be opened.
             */
            inline int open_for_reading(const std::string& filename) {
                if (filename.empty() || filename == "-") {
                    return 0; // stdin
                }

                int flags = O_RDONLY;
#ifdef _WIN32
                flags |= O_BINARY;
#endif
                const int fd = ::open(filename.c_str(), flags);
                if (fd < 0) {
                    throw std::system_error{errno, std::system_category(), std::string("Open failed for '") + filename + "'"};
                }
                return fd;
            }

            /**
             * Writes the given number of bytes from the output_buffer to the file descriptor.
             * This is just a wrapper around write(2), because write(2) can write less than
             * the given number of bytes.
             *
             * @param fd File descriptor.
             * @param output_buffer Buffer with data to be written. Must be at least size bytes long.
             * @param size Number of bytes to write.
             * @throws std::system_error On error.
             */
            inline void reliable_write(const int fd, const unsigned char* output_buffer, const size_t size) {
                constexpr size_t max_write = 100L * 1024L * 1024L; // Max 100 MByte per write
                size_t offset = 0;
                do {
                    auto write_count = size - offset;
                    if (write_count > max_write) {
                        write_count = max_write;
                    }

                    int64_t length = 0;
                    do {
                        length = ::write(fd, output_buffer + offset, static_cast<unsigned int>(write_count));
                        if (length < 0 && errno != EINTR) {
                            throw std::system_error{errno, std::system_category(), "Write failed"};
                        }
                    } while (length < 0);
                    offset += static_cast<size_t>(length);
                } while (offset < size);
            }

            /**
             * Writes the given number of bytes from the output_buffer to the file descriptor.
             * This is just a wrapper around write(2), because write(2) can write less than
             * the given number of bytes.
             *
             * @param fd File descriptor.
             * @param output_buffer Buffer with data to be written. Must be at least size bytes long.
             * @param size Number of bytes to write.
             * @throws std::system_error On error.
             */
            inline void reliable_write(const int fd, const char* output_buffer, const size_t size) {
                reliable_write(fd, reinterpret_cast<const unsigned char*>(output_buffer), size);
            }

            /**
             * Reads a maximum of size bytes from the file descriptor into the
             * input_buffer. This is just a wrapper around read(2) catching
             * errors.
             *
             * @param fd File descriptor.
             * @param input_buffer Buffer for data to be read. Must be at least size bytes long.
             * @param size Maximum number of bytes to read.
             * @returns the number of bytes read
             * @throws std::system_error On error.
             */
            inline int64_t reliable_read(const int fd, char* input_buffer, const unsigned int size) {
                int64_t nread = 0;

                do {
                    nread = ::read(fd, input_buffer, size);
                    if (nread < 0 && errno != EINTR) {
                        throw std::system_error{errno, std::system_category(), "Read failed"};
                    }
                } while (nread < 0);

                return nread;
            }

            inline void reliable_fsync(const int fd) {
#ifdef _WIN32
                if (_commit(fd) != 0) {
#else
                if (::fsync(fd) != 0) {
#endif
                    throw std::system_error{errno, std::system_category(), "Fsync failed"};
                }
            }

            inline void reliable_close(const int fd) {
                if (::close(fd) != 0) {
                    throw std::system_error{errno, std::system_category(), "Close failed"};
                }
            }

        } // namespace detail

    } // namespace io

} // namespace osmium

#endif // OSMIUM_IO_DETAIL_READ_WRITE_HPP
