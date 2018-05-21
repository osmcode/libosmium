#ifndef OSMIUM_IO_GZIP_COMPRESSION_HPP
#define OSMIUM_IO_GZIP_COMPRESSION_HPP

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

/**
 * @file
 *
 * Include this file if you want to read or write gzip-compressed OSM XML
 * files.
 *
 * @attention If you include this file, you'll need to link with `libz`.
 */

#include <osmium/io/compression.hpp>
#include <osmium/io/detail/read_write.hpp>
#include <osmium/io/error.hpp>
#include <osmium/io/file_compression.hpp>
#include <osmium/io/writer_options.hpp>
#include <osmium/util/compatibility.hpp>

#include <zlib.h>

#include <cassert>
#include <cerrno>
#include <cstddef>
#include <limits>
#include <string>

#ifndef _MSC_VER
# include <unistd.h>
#endif

namespace osmium {

    /**
     * Exception thrown when there are problems compressing or
     * decompressing gzip files.
     */
    struct gzip_error : public io_error {

        int gzip_error_code;
        int system_errno;

        gzip_error(const std::string& what, int error_code) :
            io_error(what),
            gzip_error_code(error_code),
            system_errno(error_code == Z_ERRNO ? errno : 0) {
        }

    }; // struct gzip_error

    namespace io {

        namespace detail {

            OSMIUM_NORETURN inline void throw_gzip_error(gzFile gzfile, const char* msg, int zlib_error = 0) {
                std::string error{"gzip error: "};
                error += msg;
                error += ": ";
                int errnum = zlib_error;
                if (zlib_error) {
                    error += std::to_string(zlib_error);
                } else {
                    error += ::gzerror(gzfile, &errnum);
                }
                throw osmium::gzip_error{error, errnum};
            }

        } // namespace detail

        class GzipCompressor : public Compressor {

            int m_fd;
            gzFile m_gzfile;

        public:

            explicit GzipCompressor(int fd, fsync sync) :
                Compressor(sync),
                m_fd(::dup(fd)),
                m_gzfile(::gzdopen(fd, "w")) {
                if (!m_gzfile) {
                    detail::throw_gzip_error(m_gzfile, "write initialization failed");
                }
            }

            GzipCompressor(const GzipCompressor&) = delete;
            GzipCompressor& operator=(const GzipCompressor&) = delete;

            GzipCompressor(GzipCompressor&&) = delete;
            GzipCompressor& operator=(GzipCompressor&&) = delete;

            ~GzipCompressor() noexcept final {
                try {
                    close();
                } catch (...) {
                    // Ignore any exceptions because destructor must not throw.
                }
            }

            void write(const std::string& data) final {
                if (!data.empty()) {
                    assert(data.size() < std::numeric_limits<unsigned int>::max());
                    const int nwrite = ::gzwrite(m_gzfile, data.data(), static_cast<unsigned int>(data.size()));
                    if (nwrite == 0) {
                        detail::throw_gzip_error(m_gzfile, "write failed");
                    }
                }
            }

            void close() final {
                if (m_gzfile) {
                    const int result = ::gzclose(m_gzfile);
                    m_gzfile = nullptr;
                    if (result != Z_OK) {
                        detail::throw_gzip_error(m_gzfile, "write close failed", result);
                    }
                    if (do_fsync()) {
                        osmium::io::detail::reliable_fsync(m_fd);
                    }
                    osmium::io::detail::reliable_close(m_fd);
                }
            }

        }; // class GzipCompressor

        class GzipDecompressor : public Decompressor {

            gzFile m_gzfile;

        public:

            explicit GzipDecompressor(int fd) :
                m_gzfile(::gzdopen(fd, "r")) {
                if (!m_gzfile) {
                    detail::throw_gzip_error(m_gzfile, "read initialization failed");
                }
            }

            GzipDecompressor(const GzipDecompressor&) = delete;
            GzipDecompressor& operator=(const GzipDecompressor&) = delete;

            GzipDecompressor(GzipDecompressor&&) = delete;
            GzipDecompressor& operator=(GzipDecompressor&&) = delete;

            ~GzipDecompressor() noexcept final {
                try {
                    close();
                } catch (...) {
                    // Ignore any exceptions because destructor must not throw.
                }
            }

            std::string read() final {
                std::string buffer(osmium::io::Decompressor::input_buffer_size, '\0');
                assert(buffer.size() < std::numeric_limits<unsigned int>::max());
                int nread = ::gzread(m_gzfile, const_cast<char*>(buffer.data()), static_cast<unsigned int>(buffer.size()));
                if (nread < 0) {
                    detail::throw_gzip_error(m_gzfile, "read failed");
                }
                buffer.resize(static_cast<std::string::size_type>(nread));
#if ZLIB_VERNUM >= 0x1240
                set_offset(size_t(::gzoffset(m_gzfile)));
#endif
                return buffer;
            }

            void close() final {
                if (m_gzfile) {
                    const int result = ::gzclose(m_gzfile);
                    m_gzfile = nullptr;
                    if (result != Z_OK) {
                        detail::throw_gzip_error(m_gzfile, "read close failed", result);
                    }
                }
            }

        }; // class GzipDecompressor

        class GzipBufferDecompressor : public Decompressor {

            const char* m_buffer;
            size_t m_buffer_size;
            z_stream m_zstream;

        public:

            GzipBufferDecompressor(const char* buffer, size_t size) :
                m_buffer(buffer),
                m_buffer_size(size),
                m_zstream() {
                m_zstream.next_in = reinterpret_cast<unsigned char*>(const_cast<char*>(buffer));
                assert(size < std::numeric_limits<unsigned int>::max());
                m_zstream.avail_in = static_cast<unsigned int>(size);
                const int result = inflateInit2(&m_zstream, MAX_WBITS | 32); // NOLINT(hicpp-signed-bitwise)
                if (result != Z_OK) {
                    std::string message{"gzip error: decompression init failed: "};
                    if (m_zstream.msg) {
                        message.append(m_zstream.msg);
                    }
                    throw osmium::gzip_error{message, result};
                }
            }

            GzipBufferDecompressor(const GzipBufferDecompressor&) = delete;
            GzipBufferDecompressor& operator=(const GzipBufferDecompressor&) = delete;

            GzipBufferDecompressor(GzipBufferDecompressor&&) = delete;
            GzipBufferDecompressor& operator=(GzipBufferDecompressor&&) = delete;

            ~GzipBufferDecompressor() noexcept final {
                try {
                    close();
                } catch (...) {
                    // Ignore any exceptions because destructor must not throw.
                }
            }

            std::string read() final {
                std::string output;

                if (m_buffer) {
                    const size_t buffer_size = 10240;
                    output.append(buffer_size, '\0');
                    m_zstream.next_out = reinterpret_cast<unsigned char*>(const_cast<char*>(output.data()));
                    m_zstream.avail_out = buffer_size;
                    const int result = inflate(&m_zstream, Z_SYNC_FLUSH);

                    if (result != Z_OK) {
                        m_buffer = nullptr;
                        m_buffer_size = 0;
                    }

                    if (result != Z_OK && result != Z_STREAM_END) {
                        std::string message("gzip error: inflate failed: ");
                        if (m_zstream.msg) {
                            message.append(m_zstream.msg);
                        }
                        throw osmium::gzip_error{message, result};
                    }

                    output.resize(static_cast<std::size_t>(m_zstream.next_out - reinterpret_cast<const unsigned char*>(output.data())));
                }

                return output;
            }

            void close() final {
                inflateEnd(&m_zstream);
            }

        }; // class GzipBufferDecompressor

        namespace detail {

            // we want the register_compression() function to run, setting
            // the variable is only a side-effect, it will never be used
            const bool registered_gzip_compression = osmium::io::CompressionFactory::instance().register_compression(osmium::io::file_compression::gzip,
                [](int fd, fsync sync) { return new osmium::io::GzipCompressor{fd, sync}; },
                [](int fd) { return new osmium::io::GzipDecompressor{fd}; },
                [](const char* buffer, size_t size) { return new osmium::io::GzipBufferDecompressor{buffer, size}; }
            );

            // dummy function to silence the unused variable warning from above
            inline bool get_registered_gzip_compression() noexcept {
                return registered_gzip_compression;
            }

        } // namespace detail

    } // namespace io

} // namespace osmium

#endif // OSMIUM_IO_GZIP_COMPRESSION_HPP
