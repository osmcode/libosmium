#ifndef MAPBOX_UTIL_PBF_COMMON_HPP
#define MAPBOX_UTIL_PBF_COMMON_HPP

#include <cstdint>

namespace mapbox { namespace util {

    /**
     * The type used for field tags (field numbers).
     */
    typedef uint32_t pbf_tag_type;

    /**
     * The type used to encode type information.
     * See the table on
     *    https://developers.google.com/protocol-buffers/docs/encoding
     */
    enum class pbf_wire_type : uint32_t {
        varint           = 0, // int32/64, uint32/64, sint32/64, bool, enum
        fixed64          = 1, // fixed64, sfixed64, double
        length_delimited = 2, // string, bytes, embedded messages,
                              // packed repeated fields
        fixed32          = 5, // fixed32, sfixed32, float
        unknown          = 99 // used for default setting in this library
    };

    /**
     * The type used for length values, such as the length of a field.
     */
    typedef uint32_t pbf_length_type;

}} // end namespace mapbox::util

#endif // MAPBOX_UTIL_PBF_COMMON_HPP
