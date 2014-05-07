#ifndef OSMIUM_INDEX_MAP_HPP
#define OSMIUM_INDEX_MAP_HPP

/*

This file is part of Osmium (http://osmcode.org/libosmium).

Copyright 2013 Jochen Topf <jochen@topf.org> and others (see README).

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

#include <cstddef>
#include <type_traits>

namespace osmium {

    /**
     * @brief Namespace for classes indexing OSM data.
     */
    namespace index {

        namespace map {

            /**
             * This abstract class defines an interface to storage classes
             * intended for storing small pieces of data (such as coordinates)
             * indexed by a positive integer (such as an object ID). The
             * storage must be very space efficient and able to scale to billions
             * of objects.
             *
             * Subclasses have different implementations that store the
             * data in different ways in memory and/or on disk. Some storage
             * classes are better suited when working with the whole planet,
             * some are better for data extracts.
             *
             * Note that these classes are not required to track "empty" fields.
             * When reading data you have to be sure you have put something in
             * there before.
             *
             * A typical use for this and derived classes is storage of node
             * locations indexed by node ID. These indexes will only work
             * on 64 bit systems if used in this case. 32 bit systems just
             * can't address that much memory!
             *
             * @tparam TKey Key type, usually osmium::object_id_type, must be
             *              an integral type.
             * @tparam TValue Value type, usually osmium::Location or size_t.
             *                Copied by value, so must be small type.
             */
            template <typename TKey, typename TValue>
            class Map {

                static_assert(std::is_integral<TKey>::value && std::is_unsigned<TKey>::value,
                              "TKey template parameter for class Map must be unsigned integral type");

                Map(const Map&) = delete;
                Map& operator=(const Map&) = delete;

            protected:

                Map(Map&&) = default;
                Map& operator=(Map&&) = default;

            public:

                typedef TKey key_type;
                typedef TValue mapped_type;

                Map() = default;

// workaround for a bug in GCC 4.7
#if __GNUC__ == 4 && __GNUC_MINOR__ < 8
                virtual ~Map() {}
#else
                virtual ~Map() = default;
#endif

                /// The "value" type, usually a coordinates class or similar.
                typedef TValue value_type;

                virtual void reserve(const size_t) {
                    // default implementation is empty
                }

                /// Set the field with id to value.
                virtual void set(const TKey id, const TValue value) = 0;

                /// Retrieve value by key. Does not check for overflow or empty fields.
                virtual const TValue get(const TKey id) const = 0;

                /**
                 * Get the approximate number of items in the storage. The storage
                 * might allocate memory in blocks, so this size might not be
                 * accurate. You can not use this to find out how much memory the
                 * storage uses. Use used_memory() for that.
                 */
                virtual size_t size() const = 0;

                /**
                 * Get the memory used for this storage in bytes. Note that this
                 * is not necessarily entirely accurate but an approximation.
                 * For storage classes that store the data in memory, this is
                 * the main memory used, for storage classes storing data on disk
                 * this is the memory used on disk.
                 */
                virtual size_t used_memory() const = 0;

                /**
                 * Clear memory used for this storage. After this you can not
                 * use the storage container any more.
                 */
                virtual void clear() = 0;

                /**
                 * Sort data in map. Call this after writing all data and
                 * before reading. Not all implementations need this.
                 */
                virtual void sort() {
                    // default implementation is empty
                }

            }; // class Map

        } // namespace map

    } // namespace index

} // namespace osmium

#endif // OSMIUM_INDEX_MAP_HPP
