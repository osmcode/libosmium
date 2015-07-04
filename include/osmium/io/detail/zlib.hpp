#ifndef OSMIUM_IO_DETAIL_ZLIB_HPP
#define OSMIUM_IO_DETAIL_ZLIB_HPP

/*

This file is part of Osmium (http://osmcode.org/libosmium).

Copyright 2013-2015 Jochen Topf <jochen@topf.org> and others (see README).

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

#include <memory>
#include <stdexcept>
#include <string>

#include <zlib.h>

#include <osmium/util/cast.hpp>

namespace osmium {

    namespace io {

        namespace detail {

            /**
             * Compress data using zlib.
             *
             * Note that this function can not compress data larger than
             * what fits in an unsigned long, on Windows this is usually 32bit.
             *
             * @param input Data to compress.
             * @returns Compressed data.
             */
            inline std::string zlib_compress(const std::string& input) {
                unsigned long output_size = ::compressBound(osmium::static_cast_with_assert<unsigned long>(input.size()));

                std::string output(output_size, '\0');

                auto result = ::compress(
                    reinterpret_cast<unsigned char*>(const_cast<char *>(output.data())),
                    &output_size,
                    reinterpret_cast<const unsigned char*>(input.data()),
                    osmium::static_cast_with_assert<unsigned long>(input.size())
                );

                if (result != Z_OK) {
                    throw std::runtime_error(std::string("failed to compress data: ") + zError(result));
                }

                output.resize(output_size);

                return output;
            }

            /**
             * Uncompress data using zlib.
             *
             * Note that this function can not uncompress data larger than
             * what fits in an unsigned long, on Windows this is usually 32bit.
             *
             * @param input Compressed input data.
             * @param raw_size Size of uncompressed data.
             * @returns Uncompressed data.
             */
            inline std::unique_ptr<std::string> zlib_uncompress(const std::string& input, unsigned long raw_size) {
                auto output = std::unique_ptr<std::string>(new std::string(raw_size, '\0'));

                auto result = ::uncompress(
                    reinterpret_cast<unsigned char*>(const_cast<char *>(output->data())),
                    &raw_size,
                    reinterpret_cast<const unsigned char*>(input.data()),
                    osmium::static_cast_with_assert<unsigned long>(input.size())
                );

                if (result != Z_OK) {
                    throw std::runtime_error(std::string("failed to uncompress data: ") + zError(result));
                }

                return output;
            }

            inline std::string zlib_uncompress_string(const std::string& input, unsigned long raw_size) {
                std::string output(raw_size, '\0');

                auto result = ::uncompress(
                    reinterpret_cast<unsigned char*>(const_cast<char *>(output.data())),
                    &raw_size,
                    reinterpret_cast<const unsigned char*>(input.data()),
                    osmium::static_cast_with_assert<unsigned long>(input.size())
                );

                if (result != Z_OK) {
                    throw std::runtime_error(std::string("failed to uncompress data: ") + zError(result));
                }

                return output;
            }

        } // namespace detail

    } // namespace io

} // namespace osmium

#endif // OSMIUM_IO_DETAIL_ZLIB_HPP
