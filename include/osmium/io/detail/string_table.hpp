#ifndef OSMIUM_IO_DETAIL_STRING_TABLE_HPP
#define OSMIUM_IO_DETAIL_STRING_TABLE_HPP

/*

This file is part of Osmium (https://osmcode.org/libosmium).

Copyright 2013-2019 Jochen Topf <jochen@topf.org> and others (see README).

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

#include <osmium/io/detail/pbf.hpp>

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iterator>
#include <list>
#include <string>
#include <unordered_map>
#include <utility>

namespace osmium {

    namespace io {

        namespace detail {

            /**
             * class StringStore
             *
             * Storage of lots of strings (const char *). Memory is allocated in chunks.
             * If a string is added and there is no space in the current chunk, a new
             * chunk will be allocated. Strings added to the store must not be larger
             * than the chunk size.
             *
             * All memory is released when the destructor is called. There is no other way
             * to release all or part of the memory.
             *
             */
            class StringStore {

                size_t m_chunk_size;

                std::list<std::string> m_chunks;

                void add_chunk() {
                    m_chunks.emplace_back();
                    m_chunks.back().reserve(m_chunk_size);
                }

            public:

                explicit StringStore(size_t chunk_size) :
                    m_chunk_size(chunk_size) {
                    add_chunk();
                }

                void clear() noexcept {
                    assert(!m_chunks.empty());
                    m_chunks.erase(std::next(m_chunks.begin()), m_chunks.end());
                    m_chunks.front().clear();
                }

                /**
                 * Add a null terminated string to the store. This will
                 * automatically get more memory if we are out.
                 * Returns a pointer to the copy of the string we have
                 * allocated.
                 */
                const char* add(const char* string) {
                    const size_t len = std::strlen(string) + 1;

                    assert(len <= m_chunk_size);

                    size_t chunk_len = m_chunks.back().size();
                    if (chunk_len + len > m_chunks.back().capacity()) {
                        add_chunk();
                        chunk_len = 0;
                    }

                    m_chunks.back().append(string);
                    m_chunks.back().append(1, '\0');

                    return m_chunks.back().c_str() + chunk_len;
                }

                class const_iterator {

                    using it_type = std::list<std::string>::const_iterator;

                    it_type m_it;
                    const it_type m_last;
                    const char* m_pos;

                public:

                    using iterator_category = std::forward_iterator_tag;
                    using value_type        = const char*;
                    using difference_type   = std::ptrdiff_t;
                    using pointer           = value_type*;
                    using reference         = value_type&;

                    const_iterator(it_type it, it_type last) :
                        m_it(it),
                        m_last(last),
                        m_pos(it == last ? nullptr : m_it->c_str()) {
                    }

                    const_iterator& operator++() {
                        assert(m_it != m_last);
                        const auto last_pos = m_it->c_str() + m_it->size();
                        while (m_pos != last_pos && *m_pos) {
                            ++m_pos;
                        }
                        if (m_pos != last_pos) {
                            ++m_pos;
                        }
                        if (m_pos == last_pos) {
                            ++m_it;
                            if (m_it != m_last) {
                                m_pos = m_it->c_str();
                            } else {
                                m_pos = nullptr;
                            }
                        }
                        return *this;
                    }

                    const_iterator operator++(int) {
                        const_iterator tmp{*this};
                        operator++();
                        return tmp;
                    }

                    bool operator==(const const_iterator& rhs) const {
                        return m_it == rhs.m_it && m_pos == rhs.m_pos;
                    }

                    bool operator!=(const const_iterator& rhs) const {
                        return !(*this == rhs);
                    }

                    const char* operator*() const {
                        assert(m_it != m_last);
                        assert(m_pos != nullptr);
                        return m_pos;
                    }

                }; // class const_iterator

                const_iterator begin() const {
                    if (m_chunks.front().empty()) {
                        return end();
                    }
                    return {m_chunks.begin(), m_chunks.end()};
                }

                const_iterator end() const {
                    return {m_chunks.end(), m_chunks.end()};
                }

                // These functions get you some idea how much memory was
                // used.
                size_t get_chunk_size() const noexcept {
                    return m_chunk_size;
                }

                size_t get_chunk_count() const noexcept {
                    return m_chunks.size();
                }

                size_t get_used_bytes_in_last_chunk() const noexcept {
                    return m_chunks.back().size();
                }

            }; // class StringStore

            struct str_equal {

                bool operator()(const char* lhs, const char* rhs) const noexcept {
                    return lhs == rhs || std::strcmp(lhs, rhs) == 0;
                }

            }; // struct str_equal

            struct djb2_hash {

                std::size_t operator()(const char* str) const noexcept {
                    std::size_t hash = 5381;
                    int c;

                    while ((c = *str++)) {
                        hash = ((hash << 5U) + hash) + c; /* hash * 33 + c */
                    }

                    return hash;
                }

            }; // struct djb2_hash

            class StringTable {

                // This is the maximum number of entries in a string table.
                // This should never be reached in practice but we better
                // make sure it doesn't. If we had max_uncompressed_blob_size
                // many entries, we are sure they would never fit into a PBF
                // Blob.
                enum {
                    max_entries = static_cast<int32_t>(max_uncompressed_blob_size)
                };

                // There is one string table per PBF primitive block. Most of
                // them are really small, because most blocks are full of nodes
                // with no tags. But string tables can get really large for
                // ways with many tags or for large relations.
                // The chosen size is enough so that 99% of all string tables
                // in typical OSM files will only need a single memory
                // allocation.
                enum {
                    default_stringtable_chunk_size = 100u * 1024u
                };

                StringStore m_strings;
                std::unordered_map<const char*, int32_t, djb2_hash, str_equal> m_index;
                int32_t m_size = 0;

            public:

                explicit StringTable(size_t size = default_stringtable_chunk_size) :
                    m_strings(size) {
                    m_strings.add("");
                }

                void clear() {
                    m_strings.clear();
                    m_index.clear();
                    m_size = 0;
                    m_strings.add("");
                }

                int32_t size() const noexcept {
                    return m_size + 1;
                }

                int32_t add(const char* s) {
                    const auto f = m_index.find(s);
                    if (f != m_index.end()) {
                        return f->second;
                    }

                    const char* cs = m_strings.add(s);
                    m_index[cs] = ++m_size;

                    if (m_size > max_entries) {
                        throw osmium::pbf_error{"string table has too many entries"};
                    }

                    return m_size;
                }

                StringStore::const_iterator begin() const {
                    return m_strings.begin();
                }

                StringStore::const_iterator end() const {
                    return m_strings.end();
                }

            }; // class StringTable

        } // namespace detail

    } // namespace io

} // namespace osmium

#endif // OSMIUM_IO_DETAIL_STRING_TABLE_HPP
