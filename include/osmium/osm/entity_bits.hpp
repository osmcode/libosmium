#ifndef OSMIUM_OSM_ENTITY_BITS_HPP
#define OSMIUM_OSM_ENTITY_BITS_HPP

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

#include <osmium/osm/item_type.hpp>

#include <cassert>
#include <type_traits>

namespace osmium {

    /**
     * @brief Bit field for OSM entity types.
     */
    namespace osm_entity_bits {

        /**
         * Describes zero or more OSM entities.
         *
         * Usage:
         *
         * @code{.cpp}
         * osmium::osm_entity_bits::type entities = osmium::osm_entity_bits::node | osmium::osm_entity_bits::way;
         *
         * entities |= osmium::osm_entity_bits::relation;
         *
         * assert(entities & osmium::osm_entity_bits::object);
         *
         * assert(! (entities & osmium::osm_entity_bits::changeset));
         * @endcode
         */
        enum type : unsigned char { // this should have been an enum class
                                    // but now we can't change it any more
                                    // without breaking lots of code

            nothing   = 0x00,
            node      = 0x01,
            way       = 0x02,
            nw        = 0x03, ///< node or way object
            relation  = 0x04,
            nr        = 0x05, ///< node or relation object
            wr        = 0x06, ///< way or relation object
            nwr       = 0x07, ///< node, way, or relation object
            area      = 0x08,
            na        = 0x09, ///< node or area object
            wa        = 0x0a, ///< way or area object
            nwa       = 0x0b, ///< node, way or area object
            ra        = 0x0c, ///< relation or area object
            nra       = 0x0d, ///< node, relation or area object
            wra       = 0x0e, ///< way, relation or area object
            nwra      = 0x0f, ///< node, way, relation, or area object
            object    = 0x0f, ///< node, way, relation, or area object
            changeset = 0x10,
            all       = 0x1f ///< object or changeset

        }; // enum type

        constexpr type operator|(const type lhs, const type rhs) noexcept {
            return static_cast<type>(static_cast<unsigned char>(lhs) | static_cast<unsigned char>(rhs));
        }

        constexpr type operator&(const type lhs, const type rhs) noexcept {
            return static_cast<type>(static_cast<unsigned char>(lhs) & static_cast<unsigned char>(rhs));
        }

        constexpr type operator~(const type value) noexcept {
            return all & static_cast<type>(~static_cast<unsigned char>(value));
        }

        inline type& operator|=(type& lhs, const type rhs) noexcept {
            lhs = lhs | rhs;
            return lhs;
        }

        inline type operator&=(type& lhs, const type rhs) noexcept {
            lhs = lhs & rhs;
            return lhs;
        }

        /**
         * Get entity_bits from item_type.
         *
         * @pre item_type must be undefined, node, way, relation, area, or
         *      changeset.
         */
        inline type from_item_type(osmium::item_type item_type) noexcept {
            const auto ut = static_cast<std::underlying_type_t<osmium::item_type>>(item_type);
            assert(ut <= 0x05);
            if (ut == 0) {
                return nothing;
            }
            return static_cast<osmium::osm_entity_bits::type>(1U << (ut - 1U));
        }

    } // namespace osm_entity_bits

} // namespace osmium

#endif // OSMIUM_OSM_ENTITY_BITS_HPP
