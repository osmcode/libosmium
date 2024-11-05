#ifndef OSMIUM_IO_DETAIL_PBF_INPUT_FORMAT_HPP
#define OSMIUM_IO_DETAIL_PBF_INPUT_FORMAT_HPP

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

#include <osmium/io/detail/input_format.hpp>
#include <osmium/io/detail/pbf.hpp> // IWYU pragma: export
#include <osmium/io/detail/pbf_decoder.hpp>
#include <osmium/io/detail/protobuf_tags.hpp>
#include <osmium/io/detail/read_write.hpp>
#include <osmium/io/file_format.hpp>
#include <osmium/io/header.hpp>
#include <osmium/osm/entity_bits.hpp>
#include <osmium/thread/pool.hpp>
#include <osmium/thread/util.hpp>
#include <osmium/util/config.hpp>

#include <protozero/pbf_message.hpp>
#include <protozero/types.hpp>

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>

namespace osmium {

    namespace io {

        namespace detail {

            class PBFParser final : public Parser {

                std::string m_input_buffer{};
                std::atomic<std::size_t>* m_offset_ptr;
                int m_fd;
                bool m_want_buffered_pages_removed;

                /**
                 * Make sure the input data contains at least the specified
                 * number of bytes.
                 *
                 * @param size Number of bytes to read
                 */
                void ensure_available_in_input_queue(size_t size) {
                    assert(m_fd == -1);
                    if (m_input_buffer.size() < size) {
                        m_input_buffer.reserve(size);
                    }
                    while (m_input_buffer.size() < size) {
                        const std::string new_data{get_input()};
                        if (input_done()) {
                            throw osmium::pbf_error{"truncated data (EOF encountered)"};
                        }
                        m_input_buffer += new_data;
                    }
                }

                /**
                 * Removes the specified number of bytes from the input data.
                 *
                 * @param size Number of bytes to remove
                 */
                void pop_from_input_queue(size_t size) {
                    assert(m_fd == -1);
                    m_input_buffer.erase(0, size);
                }

                static uint32_t get_size_in_network_byte_order(const char* d) noexcept {
                    return (static_cast<uint32_t>(d[3])) |
                           (static_cast<uint32_t>(d[2]) <<  8U) |
                           (static_cast<uint32_t>(d[1]) << 16U) |
                           (static_cast<uint32_t>(d[0]) << 24U);
                }

                static uint32_t check_size(uint32_t size) {
                    if (size > static_cast<uint32_t>(max_blob_header_size)) {
                        throw osmium::pbf_error{"invalid BlobHeader size (> max_blob_header_size)"};
                    }
                    return size;
                }

                /**
                 * Read 4 bytes in network byte order from file. They contain
                 * the length of the following BlobHeader.
                 */
                uint32_t read_blob_header_size_from_file() {
                    if (m_fd != -1) {
                        std::array<char, sizeof(uint32_t)> buffer{};
                        if (!osmium::io::detail::read_exactly(m_fd, buffer.data(), static_cast<unsigned int>(buffer.size()))) {
                            return 0; // EOF
                        }
                        return check_size(get_size_in_network_byte_order(buffer.data()));
                    }

                    uint32_t size = 0;

                    try {
                        ensure_available_in_input_queue(sizeof(size));
                        size = get_size_in_network_byte_order(m_input_buffer.data());
                        pop_from_input_queue(sizeof(size));
                    } catch (const osmium::pbf_error&) {
                        return 0; // EOF
                    }

                    if (size > static_cast<uint32_t>(max_blob_header_size)) {
                        throw osmium::pbf_error{"invalid BlobHeader size (> max_blob_header_size)"};
                    }

                    return size;
                }

                /**
                 * Decode the BlobHeader. Make sure it contains the expected
                 * type. Return the size of the following Blob.
                 */
                static size_t decode_blob_header(const protozero::data_view& data, const char* expected_type) {
                    protozero::pbf_message<FileFormat::BlobHeader> pbf_blob_header{data};
                    protozero::data_view blob_header_type;
                    size_t blob_header_datasize = 0;

                    while (pbf_blob_header.next()) {
                        switch (pbf_blob_header.tag_and_type()) {
                            case protozero::tag_and_type(FileFormat::BlobHeader::required_string_type, protozero::pbf_wire_type::length_delimited):
                                blob_header_type = pbf_blob_header.get_view();
                                break;
                            case protozero::tag_and_type(FileFormat::BlobHeader::required_int32_datasize, protozero::pbf_wire_type::varint):
                                blob_header_datasize = pbf_blob_header.get_int32();
                                break;
                            default:
                                pbf_blob_header.skip();
                        }
                    }

                    if (blob_header_datasize == 0) {
                        throw osmium::pbf_error{"PBF format error: BlobHeader.datasize missing or zero."};
                    }

                    if (std::strncmp(expected_type, blob_header_type.data(), blob_header_type.size()) != 0) {
                        throw osmium::pbf_error{"blob does not have expected type (OSMHeader in first blob, OSMData in following blobs)"};
                    }

                    return blob_header_datasize;
                }

                size_t check_type_and_get_blob_size(const char* expected_type) {
                    assert(expected_type);

                    const auto size = read_blob_header_size_from_file();
                    if (size == 0) { // EOF
                        return 0;
                    }

                    if (m_fd != -1) {
                        auto const buffer = read_from_input_queue_with_check(size);
                        const auto blob_size = decode_blob_header(protozero::data_view{buffer.data(), size}, expected_type);
                        return blob_size;
                    }

                    ensure_available_in_input_queue(size);
                    const auto blob_size = decode_blob_header(protozero::data_view{m_input_buffer.data(), size}, expected_type);
                    pop_from_input_queue(size);
                    return blob_size;
                }

                std::string read_from_input_queue_with_check(size_t size) {
                    if (size > max_uncompressed_blob_size) {
                        throw osmium::pbf_error{std::string{"invalid blob size: "} +
                                                std::to_string(size)};
                    }

                    std::string buffer;
                    if (m_fd != -1) {
                        buffer.resize(size);

                        if (!osmium::io::detail::read_exactly(m_fd, &*buffer.begin(), static_cast<unsigned int>(size))) {
                            throw osmium::pbf_error{"unexpected EOF"};
                        }
                    } else {
                        ensure_available_in_input_queue(size);
                        buffer.append(m_input_buffer, 0, size);
                        pop_from_input_queue(size);
                    }

                    return buffer;
                }

                // Parse the header in the PBF OSMHeader blob.
                void parse_header_blob() {
                    const auto size = check_type_and_get_blob_size("OSMHeader");
                    const osmium::io::Header header{decode_header(read_from_input_queue_with_check(size))};
                    set_header_value(header);
                }

                void parse_data_blobs() {
                    const bool use_pool = osmium::config::use_pool_threads_for_pbf_parsing();
                    while (const auto size = check_type_and_get_blob_size("OSMData")) {
                        std::string input_buffer{read_from_input_queue_with_check(size)};

                        PBFDataBlobDecoder data_blob_parser{std::move(input_buffer), read_types(), read_metadata()};

                        if (use_pool) {
                            send_to_output_queue(get_pool().submit(std::move(data_blob_parser)));
                        } else {
                            send_to_output_queue(data_blob_parser());
                        }

                        if (m_want_buffered_pages_removed) {
                            osmium::io::detail::remove_buffered_pages(m_fd, *m_offset_ptr);
                        }
                    }
                }

            public:

                explicit PBFParser(parser_arguments& args) :
                    Parser(args),
                    m_offset_ptr(args.offset_ptr),
                    m_fd(args.fd),
                    m_want_buffered_pages_removed(args.want_buffered_pages_removed) {
                }

                PBFParser(const PBFParser&) = delete;
                PBFParser& operator=(const PBFParser&) = delete;

                PBFParser(PBFParser&&) = delete;
                PBFParser& operator=(PBFParser&&) = delete;

                ~PBFParser() noexcept override = default;

                void run() override {
                    osmium::thread::set_thread_name("_osmium_pbf_in");

                    parse_header_blob();

                    if (read_types() != osmium::osm_entity_bits::nothing) {
                        parse_data_blobs();
                    }

                    osmium::io::detail::reliable_close(m_fd);
                }

            }; // class PBFParser

            // we want the register_parser() function to run, setting
            // the variable is only a side-effect, it will never be used
            const bool registered_pbf_parser = ParserFactory::instance().register_parser(
                file_format::pbf,
                [](parser_arguments& args) {
                    return std::unique_ptr<Parser>(new PBFParser{args});
            });

            // dummy function to silence the unused variable warning from above
            inline bool get_registered_pbf_parser() noexcept {
                return registered_pbf_parser;
            }

        } // namespace detail

    } // namespace io

} // namespace osmium

#endif // OSMIUM_IO_DETAIL_PBF_INPUT_FORMAT_HPP
