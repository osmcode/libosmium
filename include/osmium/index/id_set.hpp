#ifndef OSMIUM_INDEX_ID_SET_HPP
#define OSMIUM_INDEX_ID_SET_HPP

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

#include <osmium/osm/item_type.hpp>
#include <osmium/osm/types.hpp>

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstring>
#include <iterator>
#include <memory>
#include <type_traits>
#include <vector>

namespace osmium {

    namespace index {

        /**
         * Virtual parent class for IdSets. Use one of the implementations
         * provided.
         */
        template <typename T>
        class IdSet {

        public:

            IdSet() = default;

            IdSet(const IdSet&) = default;
            IdSet& operator=(const IdSet&) = default;

            IdSet(IdSet&&) noexcept = default;
            IdSet& operator=(IdSet&&) noexcept = default;

            virtual ~IdSet() noexcept = default;

            /**
             * Add the given Id to the set.
             */
            virtual void set(T id) = 0;

            /**
             * Is the Id in the set?
             */
            virtual bool get(T id) const noexcept = 0;

            /**
             * Is the set empty?
             */
            virtual bool empty() const = 0;

            /**
             * Clear the set.
             */
            virtual void clear() = 0;

            /**
             * Get an estimate of the amount of memory used for the set.
             */
            virtual std::size_t used_memory() const noexcept = 0;

        }; // class IdSet

        namespace detail {

            // This value is a compromise. For node Ids it could be bigger
            // which would mean less (but larger) memory allocations. For
            // relations Ids it could be smaller, because they would all fit
            // into a smaller allocation.
            enum : std::size_t {
                default_chunk_bits = 22U
            };

        } // namespace detail

        template <typename T, std::size_t chunk_bits = detail::default_chunk_bits>
        class IdSetDense;

        /**
         * Const_iterator for iterating over a IdSetDense.
         */
        template <typename T, std::size_t chunk_bits>
        class IdSetDenseIterator {

            static_assert(std::is_unsigned<T>::value, "Needs unsigned type");
            static_assert(sizeof(T) >= 4, "Needs at least 32bit type");

            using id_set = IdSetDense<T, chunk_bits>;

            const id_set* m_set;
            T m_value;
            T m_last;

            void next() noexcept {
                while (m_value != m_last && !m_set->get(m_value)) {
                    const T cid = id_set::chunk_id(m_value);
                    assert(cid < m_set->m_data.size());
                    if (!m_set->m_data[cid]) {
                        m_value = (cid + 1) << (chunk_bits + 3);
                    } else {
                        const auto slot = m_set->m_data[cid][id_set::offset(m_value)];
                        if (slot == 0) {
                            m_value += 8;
                            m_value &= ~0x7ULL;
                        } else {
                            ++m_value;
                        }
                    }
                }
            }

        public:

            using iterator_category = std::forward_iterator_tag;
            using value_type        = T;
            using pointer           = value_type*;
            using reference         = value_type&;

            IdSetDenseIterator(const id_set* set, T value, T last) noexcept :
                m_set(set),
                m_value(value),
                m_last(last) {
                next();
            }

            IdSetDenseIterator& operator++() noexcept {
                if (m_value != m_last) {
                    ++m_value;
                    next();
                }
                return *this;
            }

            IdSetDenseIterator operator++(int) noexcept {
                IdSetDenseIterator tmp{*this};
                operator++();
                return tmp;
            }

            bool operator==(const IdSetDenseIterator& rhs) const noexcept {
                return m_set == rhs.m_set && m_value == rhs.m_value;
            }

            bool operator!=(const IdSetDenseIterator& rhs) const noexcept {
                return !(*this == rhs);
            }

            T operator*() const noexcept {
                assert(m_value < m_last);
                return m_value;
            }

        }; // class IdSetDenseIterator

        /**
         * A set of Ids of the given type. Internal storage is in chunks of
         * arrays used as bit fields. Internally those chunks will be allocated
         * as needed, so it works relatively efficiently with both smaller
         * and larger Id sets. If it is not used, no memory is allocated at
         * all.
         */
        template <typename T, std::size_t chunk_bits>
        class IdSetDense : public IdSet<T> {

            static_assert(std::is_unsigned<T>::value, "Needs unsigned type");
            static_assert(sizeof(T) >= 4, "Needs at least 32bit type");

            friend class IdSetDenseIterator<T, chunk_bits>;

            enum : std::size_t {
                chunk_size = 1U << chunk_bits
            };

            std::vector<std::unique_ptr<unsigned char[]>> m_data;
            T m_size = 0;

            static std::size_t chunk_id(T id) noexcept {
                return id >> (chunk_bits + 3U);
            }

            static std::size_t offset(T id) noexcept {
                return (id >> 3U) & ((1U << chunk_bits) - 1U);
            }

            static unsigned int bitmask(T id) noexcept {
                return 1U << (id & 0x7U);
            }

            T last() const noexcept {
                return static_cast<T>(m_data.size()) * chunk_size * 8;
            }

            unsigned char& get_element(T id) {
                const auto cid = chunk_id(id);
                if (cid >= m_data.size()) {
                    m_data.resize(cid + 1);
                }

                auto& chunk = m_data[cid];
                if (!chunk) {
                    chunk.reset(new unsigned char[chunk_size]);
                    ::memset(chunk.get(), 0, chunk_size);
                }

                return chunk[offset(id)];
            }

        public:

            using const_iterator = IdSetDenseIterator<T, chunk_bits>;

            friend void swap(IdSetDense& first, IdSetDense& second) noexcept {
                using std::swap;
                swap(first.m_data, second.m_data);
                swap(first.m_size, second.m_size);
            }

            IdSetDense() = default;

            IdSetDense(const IdSetDense& other) :
                IdSet<T>(other),
                m_size(other.m_size) {
                m_data.reserve(other.m_data.size());
                for (const auto& ptr : other.m_data) {
                    if (ptr) {
                        m_data.emplace_back(new unsigned char[chunk_size]);
                        ::memcpy(m_data.back().get(), ptr.get(), chunk_size);
                    } else {
                        m_data.emplace_back();
                    }
                }
            }

            IdSetDense& operator=(IdSetDense other) {
                swap(*this, other);
                return *this;
            }

            IdSetDense(IdSetDense&&) noexcept = default;

            // This should really be noexcept, but GCC 4.8 doesn't like it.
            // NOLINTNEXTLINE(hicpp-noexcept-move, performance-noexcept-move-constructor)
            IdSetDense& operator=(IdSetDense&&) = default;

            ~IdSetDense() noexcept override = default;

            /**
             * Add the Id to the set if it is not already in there.
             *
             * @param id The Id to set.
             * @returns true if the Id was added, false if it was already set.
             */
            bool check_and_set(T id) {
                auto& element = get_element(id);

                if ((element & bitmask(id)) == 0) {
                    element |= bitmask(id);
                    ++m_size;
                    return true;
                }

                return false;
            }

            /**
             * Add the given Id to the set.
             *
             * @param id The Id to set.
             */
            void set(T id) final {
                (void)check_and_set(id);
            }

            /**
             * Remove the given Id from the set.
             *
             * @param id The Id to set.
             */
            void unset(T id) {
                auto& element = get_element(id);

                if ((element & bitmask(id)) != 0) {
                    element &= ~bitmask(id);
                    --m_size;
                }
            }

            /**
             * Is the Id in the set?
             *
             * @param id The Id to check.
             */
            bool get(T id) const noexcept final {
                if (chunk_id(id) >= m_data.size()) {
                    return false;
                }
                const auto* r = m_data[chunk_id(id)].get();
                if (!r) {
                    return false;
                }
                return (r[offset(id)] & bitmask(id)) != 0;
            }

            /**
             * Is the set empty?
             */
            bool empty() const noexcept final {
                return m_size == 0;
            }

            /**
             * The number of Ids stored in the set.
             */
            T size() const noexcept {
                return m_size;
            }

            /**
             * Clear the set.
             */
            void clear() final {
                m_data.clear();
                m_size = 0;
            }

            std::size_t used_memory() const noexcept final {
                return m_data.size() * chunk_size;
            }

            const_iterator begin() const {
                return {this, 0, last()};
            }

            const_iterator end() const {
                return {this, last(), last()};
            }

        }; // class IdSetDense

        /**
         * IdSet implementation for small Id sets. It writes the Ids
         * into a vector and uses linear search.
         */
        template <typename T>
        class IdSetSmall : public IdSet<T> {

            std::vector<T> m_data;

        public:

            IdSetSmall() = default;

            IdSetSmall(const IdSetSmall&) = default;
            IdSetSmall& operator=(const IdSetSmall&) = default;

            IdSetSmall(IdSetSmall&&) noexcept = default;
            IdSetSmall& operator=(IdSetSmall&&) noexcept = default;

            ~IdSetSmall() noexcept override = default;

            /**
             * Add the given Id to the set.
             */
            void set(T id) final {
                if (m_data.empty() || m_data.back() != id) {
                    m_data.push_back(id);
                }
            }

            /**
             * Is the Id in the set? Uses linear search.
             *
             * @param id The Id to check.
             */
            bool get(T id) const noexcept final {
                const auto it = std::find(m_data.cbegin(), m_data.cend(), id);
                return it != m_data.cend();
            }

            /**
             * Is the Id in the set? Uses a binary search. For larger sets
             * this might be more efficient than calling get(), the set
             * must be sorted.
             *
             * @param id The Id to check.
             * @pre You must have called sort_unique() before calling this
             *      or be sure there are no duplicates and the Ids have been
             *      set in order.
             */
            bool get_binary_search(T id) const noexcept {
                return std::binary_search(m_data.cbegin(), m_data.cend(), id);
            }

            /**
             * Is the set empty?
             */
            bool empty() const noexcept final {
                return m_data.empty();
            }

            /**
             * Clear the set.
             */
            void clear() final {
                m_data.clear();
            }

            /**
             * Sort the internal vector and remove any duplicates. Call this
             * before using size(), get_binary_search(), merge_sorted() or
             * using an iterator.
             */
            void sort_unique() {
                std::sort(m_data.begin(), m_data.end());
                const auto last = std::unique(m_data.begin(), m_data.end());
                m_data.erase(last, m_data.end());

            }

            /**
             * The number of Ids stored in the set.
             *
             * @pre You must have called sort_unique() before calling this
             *      or be sure there are no duplicates.
             */
            std::size_t size() const noexcept {
                return m_data.size();
            }

            std::size_t used_memory() const noexcept final {
                return m_data.capacity() * sizeof(T);
            }

            /**
             * Merge the other set into this one. The result is sorted.
             *
             * @pre Both sets must be sorted and must not contain any
             *      duplicates. Call sort_unique() if you are not sure.
             */
            void merge_sorted(const IdSetSmall<T>& other) {
                std::vector<T> new_data;
                new_data.reserve(m_data.size() + other.m_data.size());
                std::set_union(m_data.cbegin(), m_data.cend(),
                               other.m_data.cbegin(), other.m_data.cend(),
                               std::back_inserter(new_data));
                using std::swap;
                swap(new_data, m_data);
            }

            /// Iterator type. There is no non-const iterator.
            using const_iterator = typename std::vector<T>::const_iterator;

            const_iterator begin() const noexcept {
                return m_data.cbegin();
            }

            const_iterator end() const noexcept {
                return m_data.cend();
            }

            const_iterator cbegin() const noexcept {
                return m_data.cbegin();
            }

            const_iterator cend() const noexcept {
                return m_data.cend();
            }

        }; // class IdSetSmall

    } // namespace index

} // namespace osmium

#endif // OSMIUM_INDEX_ID_SET_HPP
