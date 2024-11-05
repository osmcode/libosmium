#ifndef OSMIUM_OSM_TYPES_FROM_STRING_HPP
#define OSMIUM_OSM_TYPES_FROM_STRING_HPP

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

#include <osmium/osm/entity_bits.hpp>
#include <osmium/osm/item_type.hpp>
#include <osmium/osm/types.hpp>

#include <cassert>
#include <cctype>
#include <cstdlib>
#include <limits>
#include <stdexcept>
#include <string>
#include <utility>

namespace osmium {

    /**
     * Convert string with object id to object_id_type.
     *
     * @pre @code input != nullptr @endcode
     *
     * @param input Input string.
     *
     * @throws std::range_error if the value is out of range.
     */
    inline object_id_type string_to_object_id(const char* input) {
        assert(input);
        if (*input != '\0' && !std::isspace(*input)) {
            char* end = nullptr;
            const auto id = std::strtoll(input, &end, 10);
            if (id != std::numeric_limits<long long>::min() && // NOLINT(google-runtime-int)
                id != std::numeric_limits<long long>::max() && // NOLINT(google-runtime-int)
                *end == '\0') {
                return id;
            }
        }
        throw std::range_error{std::string{"illegal id: '"} + input + "'"};
    }

    /**
     * Parse string with object type identifier followed by object id. This
     * reads strings like "n1234" and "w10". If there is no type prefix,
     * the default_type is returned.
     *
     * @pre @code input != nullptr @endcode
     * @pre @code types != osmium::osm_entity_bits::nothing @endcode
     *
     * @param input Input string.
     * @param types Allowed types. Must not be osmium::osm_entity_bits::nothing.
     * @param default_type Type used when there is no type prefix.
     *
     * @returns std::pair of type and id.
     *
     * @throws std::range_error if the value is out of range.
     */
    inline std::pair<osmium::item_type, osmium::object_id_type>
    string_to_object_id(const char* input,
                        osmium::osm_entity_bits::type types,
                        osmium::item_type default_type = osmium::item_type::undefined) {
        assert(input);
        assert(types != osmium::osm_entity_bits::nothing);
        if (*input != '\0') {
            if (std::isdigit(*input)) {
                return std::make_pair(default_type, string_to_object_id(input));
            }
            const osmium::item_type t = osmium::char_to_item_type(*input);
            if (osmium::osm_entity_bits::from_item_type(t) & types) {
                return std::make_pair(t, string_to_object_id(input + 1));
            }
        }
        throw std::range_error{std::string{"not a valid id: '"} + input + "'"};
    }

    namespace detail {

        inline uint32_t string_to_ulong(const char* input, const char* name) {
            if (input[0] == '-' && input[1] == '1' && input[2] == '\0') {
                return 0;
            }
            if (*input != '\0' && *input != '-' && !std::isspace(*input)) {
                char* end = nullptr;
                const auto value = std::strtoul(input, &end, 10);
                if (value < std::numeric_limits<uint32_t>::max() && *end == '\0') {
                    return static_cast<uint32_t>(value);
                }
            }
            throw std::range_error{std::string{"illegal "} + name + ": '" + input + "'"};
        }

    } // namespace detail

    /**
     * Convert string with object version to object_version_type.
     *
     * @pre input must not be nullptr.
     *
     * @param input Input string.
     *
     * @throws std::range_error if the value is out of range.
     */
    inline object_version_type string_to_object_version(const char* input) {
        assert(input);
        return detail::string_to_ulong(input, "version");
    }

    /**
     * Convert string with changeset id to changeset_id_type.
     *
     * @pre input must not be nullptr.
     *
     * @param input Input string.
     *
     * @throws std::range_error if the value is out of range.
     */
    inline changeset_id_type string_to_changeset_id(const char* input) {
        assert(input);
        return detail::string_to_ulong(input, "changeset");
    }

    /**
     * Convert string with user id to user_id_type.
     *
     * @pre input must not be nullptr.
     *
     * @param input Input string.
     *
     * @throws std::range_error if the value is out of range.
     */
    inline user_id_type string_to_uid(const char* input) {
        assert(input);
        return detail::string_to_ulong(input, "user id");
    }

    /**
     * Convert string with number of changes to num_changes_type.
     *
     * @pre input must not be nullptr.
     *
     * @param input Input string.
     *
     * @throws std::range_error if the value is out of range.
     */
    inline num_changes_type string_to_num_changes(const char* input) {
        assert(input);
        return detail::string_to_ulong(input, "value for num changes");
    }

    /**
     * Convert string with number of comments to num_comments_type.
     *
     * @pre input must not be nullptr.
     *
     * @param input Input string.
     *
     * @throws std::range_error if the value is out of range.
     */
    inline num_comments_type string_to_num_comments(const char* input) {
        assert(input);
        return detail::string_to_ulong(input, "value for num comments");
    }

} // namespace osmium

#endif // OSMIUM_OSM_TYPES_FROM_STRING_HPP
