#ifndef OSMIUM_IO_DETAIL_DEBUG_OUTPUT_FORMAT_HPP
#define OSMIUM_IO_DETAIL_DEBUG_OUTPUT_FORMAT_HPP

/*

This file is part of Osmium (https://osmcode.org/libosmium).

Copyright 2013-2025 Jochen Topf <jochen@topf.org> and others (see README).

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

#include <osmium/io/detail/output_format.hpp>
#include <osmium/io/detail/queue_util.hpp>
#include <osmium/io/detail/string_util.hpp>
#include <osmium/io/file.hpp>
#include <osmium/io/file_format.hpp>
#include <osmium/io/header.hpp>
#include <osmium/memory/buffer.hpp>
#include <osmium/memory/item_iterator.hpp>
#include <osmium/osm/box.hpp>
#include <osmium/osm/changeset.hpp>
#include <osmium/osm/crc.hpp>
#include <osmium/osm/crc_zlib.hpp>
#include <osmium/osm/item_type.hpp>
#include <osmium/osm/location.hpp>
#include <osmium/osm/metadata_options.hpp>
#include <osmium/osm/node.hpp>
#include <osmium/osm/node_ref.hpp>
#include <osmium/osm/object.hpp>
#include <osmium/osm/relation.hpp>
#include <osmium/osm/tag.hpp>
#include <osmium/osm/timestamp.hpp>
#include <osmium/osm/types.hpp>
#include <osmium/osm/way.hpp>
#include <osmium/thread/pool.hpp>
#include <osmium/util/minmax.hpp>
#include <osmium/visitor.hpp>

#include <array>
#include <cmath>
#include <cstring>
#include <iterator>
#include <memory>
#include <string>
#include <utility>

namespace osmium {

    namespace io {

        namespace detail {

            constexpr const char* color_bold    = "\x1b[1m";
            constexpr const char* color_black   = "\x1b[30m";
            constexpr const char* color_gray    = "\x1b[30;1m";
            constexpr const char* color_red     = "\x1b[31m";
            constexpr const char* color_green   = "\x1b[32m";
            constexpr const char* color_yellow  = "\x1b[33m";
            constexpr const char* color_blue    = "\x1b[34m";
            constexpr const char* color_magenta = "\x1b[35m";
            constexpr const char* color_cyan    = "\x1b[36m";
            constexpr const char* color_white   = "\x1b[37m";

            constexpr const char* color_backg_red   = "\x1b[41m";
            constexpr const char* color_backg_green = "\x1b[42m";

            constexpr const char* color_reset   = "\x1b[0m";

            struct debug_output_options {

                /// Which metadata of objects should be added?
                osmium::metadata_options add_metadata;

                /// Output with ANSI colors?
                bool use_color = false;

                /// Add CRC32 checksum to each object?
                bool add_crc32 = false;

                /// Write in form of a diff file?
                bool format_as_diff = false;

            }; // struct debug_output_options

            /**
             * Writes out one buffer with OSM data in Debug format.
             */
            class DebugOutputBlock : public OutputBlock {

                using crc_type = osmium::CRC_zlib;

                debug_output_options m_options;

                const char* m_utf8_prefix = "";
                const char* m_utf8_suffix = "";

                char m_diff_char = '\0';

                void append_encoded_string(const char* data) {
                    append_debug_encoded_string(*m_out, data, m_utf8_prefix, m_utf8_suffix);
                }

                template <typename... TArgs>
                void output_formatted(const char* format, TArgs... args) {
                    append_printf_formatted_string(*m_out, format, args...);
                }

                void write_color(const char* color) {
                    if (m_options.use_color) {
                        *m_out += color;
                    }
                }

                void write_diff() {
                    if (!m_diff_char) {
                        return;
                    }
                    if (m_options.use_color) {
                        if (m_diff_char == '-') {
                            *m_out += color_backg_red;
                            *m_out += color_white;
                            *m_out += color_bold;
                            *m_out += '-';
                            *m_out += color_reset;
                            return;
                        }
                        if (m_diff_char == '+') {
                            *m_out += color_backg_green;
                            *m_out += color_white;
                            *m_out += color_bold;
                            *m_out += '+';
                            *m_out += color_reset;
                            return;
                        }
                    }
                    *m_out += m_diff_char;
                }

                void write_string(const char* string) {
                    *m_out += '"';
                    write_color(color_blue);
                    append_encoded_string(string);
                    write_color(color_reset);
                    *m_out += '"';
                }

                void write_object_type(const char* object_type, bool visible = true) {
                    write_diff();
                    if (visible) {
                        write_color(color_bold);
                    } else {
                        write_color(color_white);
                    }
                    *m_out += object_type;
                    write_color(color_reset);
                    *m_out += ' ';
                }

                void write_fieldname(const char* name) {
                    write_diff();
                    *m_out += "  ";
                    write_color(color_cyan);
                    *m_out += name;
                    write_color(color_reset);
                    *m_out += ": ";
                }

                void write_comment_field(const char* name) {
                    write_color(color_cyan);
                    *m_out += name;
                    write_color(color_reset);
                    *m_out += ": ";
                }

                void write_counter(int width, int n) {
                    write_color(color_white);
#pragma GCC diagnostic push
#if !defined(__clang__) && defined(__GNUC__) && (__GNUC__ > 7)
#pragma GCC diagnostic ignored "-Wformat-truncation"
#endif
                    output_formatted("    %0*d: ", width, n++);
#pragma GCC diagnostic pop
                    write_color(color_reset);
                }

                void write_error(const char* msg) {
                    write_color(color_red);
                    *m_out += msg;
                    write_color(color_reset);
                }

                void write_timestamp(const osmium::Timestamp& timestamp) {
                    if (timestamp.valid()) {
                        *m_out += timestamp.to_iso();
                        *m_out += " (";
                        output_int(timestamp.seconds_since_epoch());
                        *m_out += ')';
                    } else {
                        write_error("NOT SET");
                    }
                    *m_out += '\n';
                }

                void write_meta(const osmium::OSMObject& object) {
                    output_int(object.id());
                    if (object.visible()) {
                        *m_out += " visible\n";
                    } else {
                        write_error(" deleted\n");
                    }
                    if (m_options.add_metadata.version()) {
                        write_fieldname("version");
                        *m_out += "  ";
                        output_int(object.version());
                        *m_out += '\n';
                    }
                    if (m_options.add_metadata.changeset()) {
                        write_fieldname("changeset");
                        output_int(object.changeset());
                        *m_out += '\n';
                    }
                    if (m_options.add_metadata.timestamp()) {
                        write_fieldname("timestamp");
                        write_timestamp(object.timestamp());
                    }
                    if (m_options.add_metadata.user() || m_options.add_metadata.uid()) {
                        write_fieldname("user");
                        *m_out += "     ";
                        if (m_options.add_metadata.uid()) {
                            output_int(object.uid());
                            *m_out += ' ';
                        }
                        if (m_options.add_metadata.user()) {
                            write_string(object.user());
                        }
                        *m_out += '\n';
                    }
                }

                void write_tags(const osmium::TagList& tags, const char* padding = "") {
                    if (tags.empty()) {
                        return;
                    }
                    write_fieldname("tags");
                    *m_out += padding;
                    *m_out += "     ";
                    output_int(tags.size());
                    *m_out += '\n';

                    osmium::max_op<size_t> max;
                    for (const auto& tag : tags) {
                        max.update(std::strlen(tag.key()));
                    }
                    for (const auto& tag : tags) {
                        write_diff();
                        *m_out += "    ";
                        write_string(tag.key());
                        auto spacing = max() - std::strlen(tag.key());
                        while (spacing > 0) {
                            *m_out += " ";
                            --spacing;
                        }
                        *m_out += " = ";
                        write_string(tag.value());
                        *m_out += '\n';
                    }
                }

                void write_location(const osmium::Location& location) {
                    write_fieldname("lon/lat");
                    *m_out += "  ";
                    location.as_string_without_check(std::back_inserter(*m_out));
                    if (!location.valid()) {
                        write_error(" INVALID LOCATION!");
                    }
                    *m_out += '\n';
                }

                void write_box(const osmium::Box& box) {
                    write_fieldname("box l/b/r/t");
                    if (box.bottom_left().is_undefined() &&
                        box.top_right().is_undefined()) {
                        write_error("BOX NOT SET!\n");
                        return;
                    }
                    const auto& bl = box.bottom_left();
                    const auto& tr = box.top_right();
                    bl.as_string_without_check(std::back_inserter(*m_out));
                    *m_out += ' ';
                    tr.as_string_without_check(std::back_inserter(*m_out));
                    if (!box.valid()) {
                        write_error(" INVALID BOX!");
                    }
                    *m_out += '\n';
                }

                template <typename T>
                void write_crc32(const T& object) {
                    write_fieldname("crc32");
                    osmium::CRC<crc_type> crc32;
                    crc32.update(object);
                    output_formatted("    %x\n", crc32().checksum());
                }

                void write_crc32(const osmium::Changeset& object) {
                    write_fieldname("crc32");
                    osmium::CRC<crc_type> crc32;
                    crc32.update(object);
                    output_formatted("      %x\n", crc32().checksum());
                }

            public:

                DebugOutputBlock(osmium::memory::Buffer&& buffer, const debug_output_options& options) :
                    OutputBlock(std::move(buffer)),
                    m_options(options),
                    m_utf8_prefix(options.use_color ? color_red  : ""),
                    m_utf8_suffix(options.use_color ? color_blue : "") {
                }

                std::string operator()() {
                    osmium::apply(m_input_buffer->cbegin(), m_input_buffer->cend(), *this);

                    std::string out;
                    using std::swap;
                    swap(out, *m_out);

                    return out;
                }

                void node(const osmium::Node& node) {
                    m_diff_char = m_options.format_as_diff ? node.diff_as_char() : '\0';

                    write_object_type("node", node.visible());
                    write_meta(node);

                    if (node.visible()) {
                        write_location(node.location());
                    }

                    write_tags(node.tags());

                    if (m_options.add_crc32) {
                        write_crc32(node);
                    }

                    *m_out += '\n';
                }

                void way(const osmium::Way& way) {
                    m_diff_char = m_options.format_as_diff ? way.diff_as_char() : '\0';

                    write_object_type("way", way.visible());
                    write_meta(way);
                    write_tags(way.tags());

                    write_fieldname("nodes");

                    *m_out += "    ";
                    output_int(way.nodes().size());
                    if (way.nodes().size() < 2) {
                        write_error(" LESS THAN 2 NODES!\n");
                    } else if (way.nodes().size() > 2000) {
                        write_error(" MORE THAN 2000 NODES!\n");
                    } else if (way.nodes().is_closed()) {
                        *m_out += " (closed)\n";
                    } else {
                        *m_out += " (open)\n";
                    }

                    const int width = static_cast<int>(std::log10(way.nodes().size())) + 1;
                    int n = 0;
                    for (const auto& node_ref : way.nodes()) {
                        write_diff();
                        write_counter(width, n++);
                        output_formatted("%10lld", static_cast<long long>(node_ref.ref())); // NOLINT(google-runtime-int)
                        if (node_ref.location().valid()) {
                            *m_out += " (";
                            node_ref.location().as_string(std::back_inserter(*m_out));
                            *m_out += ')';
                        }
                        *m_out += '\n';
                    }

                    if (m_options.add_crc32) {
                        write_crc32(way);
                    }

                    *m_out += '\n';
                }

                void relation(const osmium::Relation& relation) {
                    static const std::array<const char*, 3> short_typename = {{ "node", "way ", "rel " }};

                    m_diff_char = m_options.format_as_diff ? relation.diff_as_char() : '\0';

                    write_object_type("relation", relation.visible());
                    write_meta(relation);
                    write_tags(relation.tags());

                    write_fieldname("members");
                    *m_out += "  ";
                    output_int(relation.members().size());
                    *m_out += '\n';

                    const int width = static_cast<int>(std::log10(relation.members().size())) + 1;
                    int n = 0;
                    for (const auto& member : relation.members()) {
                        write_diff();
                        write_counter(width, n++);
                        *m_out += short_typename[item_type_to_nwr_index(member.type())];
                        output_formatted(" %10lld ", static_cast<long long>(member.ref())); // NOLINT(google-runtime-int)
                        write_string(member.role());
                        *m_out += '\n';
                    }

                    if (m_options.add_crc32) {
                        write_crc32(relation);
                    }

                    *m_out += '\n';
                }

                void changeset(const osmium::Changeset& changeset) {
                    write_object_type("changeset");
                    output_int(changeset.id());
                    *m_out += '\n';

                    write_fieldname("num changes");
                    output_int(changeset.num_changes());
                    if (changeset.num_changes() == 0) {
                        write_error(" NO CHANGES!");
                    }
                    *m_out += '\n';

                    write_fieldname("created at");
                    *m_out += ' ';
                    write_timestamp(changeset.created_at());

                    write_fieldname("closed at");
                    *m_out += "  ";
                    if (changeset.closed()) {
                        write_timestamp(changeset.closed_at());
                    } else {
                        write_error("OPEN!\n");
                    }

                    write_fieldname("user");
                    *m_out += "       ";
                    output_int(changeset.uid());
                    *m_out += ' ';
                    write_string(changeset.user());
                    *m_out += '\n';

                    write_box(changeset.bounds());
                    write_tags(changeset.tags(), "  ");

                    if (changeset.num_comments() > 0) {
                        write_fieldname("comments");
                        *m_out += "   ";
                        output_int(changeset.num_comments());
                        *m_out += '\n';

                        const int width = static_cast<int>(std::log10(changeset.num_comments())) + 1;
                        int n = 0;
                        for (const auto& comment : changeset.discussion()) {
                            write_counter(width, n++);

                            write_comment_field("date");
                            write_timestamp(comment.date());
                            output_formatted("      %*s", width, "");

                            write_comment_field("user");
                            output_int(comment.uid());
                            *m_out += ' ';
                            write_string(comment.user());
                            output_formatted("\n      %*s", width, "");

                            write_comment_field("text");
                            write_string(comment.text());
                            *m_out += '\n';
                        }
                    }

                    if (m_options.add_crc32) {
                        write_crc32(changeset);
                    }

                    *m_out += '\n';
                }

            }; // class DebugOutputBlock

            class DebugOutputFormat : public osmium::io::detail::OutputFormat {

                debug_output_options m_options;

                void write_fieldname(std::string& out, const char* name) const {
                    out += "  ";
                    if (m_options.use_color) {
                        out += color_cyan;
                    }
                    out += name;
                    if (m_options.use_color) {
                        out += color_reset;
                    }
                    out += ": ";
                }

            public:

                DebugOutputFormat(osmium::thread::Pool& pool, const osmium::io::File& file, future_string_queue_type& output_queue) :
                    OutputFormat(pool, output_queue) {
                    m_options.add_metadata   = osmium::metadata_options{file.get("add_metadata")};
                    m_options.use_color      = file.is_true("color");
                    m_options.add_crc32      = file.is_true("add_crc32");
                    m_options.format_as_diff = file.is_true("diff");
                }

                void write_header(const osmium::io::Header& header) final {
                    if (m_options.format_as_diff) {
                        return;
                    }

                    std::string out;

                    if (m_options.use_color) {
                        out += color_bold;
                    }
                    out += "header\n";
                    if (m_options.use_color) {
                        out += color_reset;
                    }

                    write_fieldname(out, "multiple object versions");
                    out += header.has_multiple_object_versions() ? "yes" : "no";
                    out += '\n';
                    write_fieldname(out, "bounding boxes");
                    out += '\n';
                    for (const auto& box : header.boxes()) {
                        out += "    ";
                        box.bottom_left().as_string(std::back_inserter(out));
                        out += ' ';
                        box.top_right().as_string(std::back_inserter(out));
                        out += '\n';
                    }
                    write_fieldname(out, "options");
                    out += '\n';
                    for (const auto& opt : header) {
                        out += "    ";
                        out += opt.first;
                        out += " = ";
                        out += opt.second;
                        out += '\n';
                    }
                    out += "\n=============================================\n\n";

                    send_to_output_queue(std::move(out));
                }

                void write_buffer(osmium::memory::Buffer&& buffer) final {
                    m_output_queue.push(m_pool.submit(DebugOutputBlock{std::move(buffer), m_options}));
                }

            }; // class DebugOutputFormat

            // we want the register_output_format() function to run, setting
            // the variable is only a side-effect, it will never be used
            const bool registered_debug_output = osmium::io::detail::OutputFormatFactory::instance().register_output_format(osmium::io::file_format::debug,
                [](osmium::thread::Pool& pool, const osmium::io::File& file, future_string_queue_type& output_queue) {
                    return new osmium::io::detail::DebugOutputFormat(pool, file, output_queue);
            });

            // dummy function to silence the unused variable warning from above
            inline bool get_registered_debug_output() noexcept {
                return registered_debug_output;
            }

        } // namespace detail

    } // namespace io

} // namespace osmium

#endif // OSMIUM_IO_DETAIL_DEBUG_OUTPUT_FORMAT_HPP
