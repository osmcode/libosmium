#ifndef OSMIUM_IO_DETAIL_LZ4_HPP
#define OSMIUM_IO_DETAIL_LZ4_HPP

/*

This file is part of Osmium (https://osmcode.org/libosmium).

Copyright 2013-2024 Jochen Topf <jochen@topf.org> and others (see README).

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

#ifdef OSMIUM_WITH_LZ4

#include <cassert>
#include <limits>
#include <stdexcept>
#include <string>

#include <osmium/io/error.hpp>

#include <protozero/version.hpp>

#if PROTOZERO_VERSION_CODE >= 10600
# include <protozero/data_view.hpp>
#else
# include <protozero/types.hpp>
#endif

#include <lz4.h>

namespace osmium {

    namespace io {

        namespace detail {

            constexpr int lz4_default_compression_level() noexcept {
                return 1; // LZ4_ACCELERATION_DEFAULT
            }

            inline void lz4_check_compression_level(int value) {
                if (value <= 0 || value > 65537 /* LZ4_ACCELERATION_MAX */) {
                    throw std::invalid_argument{"The 'pbf_compression_level' for lz4 compression must be between 1 and 65537."};
                }
            }

            /**
             * Compress data using lz4.
             *
             * Note that this function can not compress data larger than
             * LZ4_MAX_INPUT_SIZE.
             *
             * @param input Data to compress.
             * @param compression_level Compression level.
             * @returns Compressed data.
             */
            inline std::string lz4_compress(const std::string& input, int compression_level = lz4_default_compression_level()) { // NOLINT(google-runtime-int)
                assert(input.size() < LZ4_MAX_INPUT_SIZE);
                const int output_size = ::LZ4_compressBound(static_cast<int>(input.size())); // NOLINT(google-runtime-int)

                std::string output(static_cast<std::size_t>(output_size), '\0');

                const int result = ::LZ4_compress_fast( // NOLINT(google-runtime-int)
                    input.data(),
                    &*output.begin(),
                    static_cast<int>(input.size()),
                    output_size,
                    compression_level);

                if (result == 0) {
                    throw io_error{"LZ4 compression failed"};
                }

                output.resize(result);

                return output;
            }

            /**
             * Uncompress data using lz4.
             *
             * Note that this function can not uncompress data larger than
             * LZ4_MAX_INPUT_SIZE.
             *
             * @param input Compressed input data.
             * @param raw_size Size of uncompressed data.
             * @param output Uncompressed result data.
             * @returns Pointer and size to incompressed data.
             */
            inline protozero::data_view lz4_uncompress_string(const char* input, unsigned long input_size, unsigned long raw_size, std::string& output) { // NOLINT(google-runtime-int)
                output.resize(raw_size);

                const int result = ::LZ4_decompress_safe( // NOLINT(google-runtime-int)
                    input,
                    &*output.begin(),
                    static_cast<int>(input_size),
                    static_cast<int>(raw_size));

                if (result < 0) {
                    throw io_error{"LZ4 decompression failed: invalid block"};
                }

                if (result != static_cast<int>(raw_size)) {
                    throw io_error{"LZ4 decompression failed: data size does not match"};
                }

                return protozero::data_view{output.data(), output.size()};
            }

        } // namespace detail

    } // namespace io

} // namespace osmium

#endif

#endif // OSMIUM_IO_DETAIL_LZ4_HPP
