#ifndef OSMIUM_INDEX_MAP_SPARSE_MEM_MAP_HPP
#define OSMIUM_INDEX_MAP_SPARSE_MEM_MAP_HPP

/*

This file is part of Osmium (https://osmcode.org/libosmium).

Copyright 2013-2021 Jochen Topf <jochen@topf.org> and others (see README).

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

#include <osmium/index/index.hpp>
#include <osmium/index/map.hpp>
#include <osmium/io/detail/read_write.hpp>

#include <algorithm> // IWYU pragma: keep (for std::copy)
#include <cstddef>
#include <iostream>
#include <iterator>
#include <map>
#include <vector>

#define OSMIUM_HAS_INDEX_MAP_SPARSE_MEM_COMPACT_ARRAY

namespace osmium {

    namespace index {

        namespace map {

            /**
             * This implementation is faster and uses less memory than SparseMemArray,
             * it is suited both for big and small maps, with bigger gains for bigger maps.
             * 
             * Performance is worse if the index is fed unordered ids.
             */

            template <typename TId, typename TValue>
            class SparseMemCompactArray : public osmium::index::map::Map<TId, TValue> {

                using index_type = typename std::pair<TId, size_t>;
                using id_value_pair_type = typename std::pair<TId, TValue>;

                TId m_last_id;
                std::vector<TValue> m_elements;
                std::vector<index_type> m_index;

                void sorted_vector_of_pairs(std::vector<id_value_pair_type>& v) {
                    v = std::vector<id_value_pair_type>(m_elements.size());
                    size_t i = -1;
                    TId id = 0;
                    for (size_t j = 0; j < m_elements.size(); ++j) {
                        id = m_index[i+1].second == j ? m_index[++i].first : id + 1;
                        v[j] = id_value_pair_type{id, m_elements[j]};
                    }

                    std::sort(v.begin(), v.end());
                }

              public:

                SparseMemCompactArray() = default;

                void set(const TId id, const TValue value) final {
                    if (m_index.empty() or id != m_last_id + 1) {
                        m_index.push_back({id, m_elements.size()});
                    }
                    m_last_id = id;
                    m_elements.push_back(value);
                }

                TValue get(const TId id) const final {
                    auto value = get_noexcept(id);
                    if (value == osmium::index::empty_value<TValue>()) {
                        throw osmium::not_found{id};
                    }
                    return value;
                }

                TValue get_noexcept(const TId id) const noexcept final {
                    auto next = std::upper_bound(
                        m_index.begin(), m_index.end(), index_type{id, 0},
                        [](const index_type &a, const index_type &b) {
                            return a.first < b.first;
                        });

                    if (next == m_index.begin()) { // First ID is greater than queried ID
                        return osmium::index::empty_value<TValue>();
                    }

                    // Expected value position in m_elements
                    auto elem_index = std::prev(next)->second + (id - std::prev(next)->first);

                    if (elem_index >= m_elements.size()) { // Position out of bounds
                        return osmium::index::empty_value<TValue>();
                    }

                    if (next != m_index.end() and elem_index >= next->second) { // Position greater than next position in index
                        return osmium::index::empty_value<TValue>();
                    }

                    return m_elements[elem_index];
                }

                size_t size() const noexcept final {
                    return m_elements.size();
                }

                size_t used_memory() const noexcept final {
                  return sizeof(TValue) * m_elements.size() +
                         sizeof(index_type) * m_index.size();
                }

                void clear() final {
                    m_elements.clear();
                    m_index.clear();
                }

                void sort() final {
                    std::vector<id_value_pair_type> v;
                    sorted_vector_of_pairs(v);

                    clear();

                    for (auto &p : v) {
                        set(p.first, p.second);
                    }
                }

                void dump_as_list(const int fd) final {
                    std::vector<id_value_pair_type> v;
                    sorted_vector_of_pairs(v);
                    osmium::io::detail::reliable_write(fd, reinterpret_cast<const char*>(v.data()), sizeof(id_value_pair_type) * v.size());
                }

            }; // class SparseMemCompactArray

        } // namespace map

    } // namespace index

} // namespace osmium

#ifdef OSMIUM_WANT_NODE_LOCATION_MAPS
    REGISTER_MAP(osmium::unsigned_object_id_type, osmium::Location, osmium::index::map::SparseMemCompactArray, sparse_mem_compact_array)
#endif

#endif // OSMIUM_HAS_INDEX_MAP_SPARSE_MEM_COMPACT_ARRAY
