#ifndef OSMIUM_INDEX_MAP_SPARSE_MMAP_ARRAY_HPP
#define OSMIUM_INDEX_MAP_SPARSE_MMAP_ARRAY_HPP

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

#ifdef __linux__

#include <osmium/index/detail/mmap_vector_anon.hpp>
#include <osmium/index/detail/vector_map.hpp>

#define OSMIUM_HAS_INDEX_MAP_SPARSE_MMAP_ARRAY

namespace osmium {

    namespace index {

        namespace map {

            template <typename TId, typename TValue>
            using SparseMmapArray = VectorBasedSparseMap<TId, TValue, osmium::detail::mmap_vector_anon>;

        } // namespace map

    } // namespace index

} // namespace osmium

#ifdef OSMIUM_WANT_NODE_LOCATION_MAPS
    REGISTER_MAP(osmium::unsigned_object_id_type, osmium::Location, osmium::index::map::SparseMmapArray, sparse_mmap_array)
#endif

#endif // __linux__

#endif // OSMIUM_INDEX_MAP_SPARSE_MMAP_ARRAY_HPP
