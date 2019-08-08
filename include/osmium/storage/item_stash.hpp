#ifndef OSMIUM_STORAGE_ITEM_STASH_HPP
#define OSMIUM_STORAGE_ITEM_STASH_HPP

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

#include <osmium/memory/buffer.hpp>
#include <osmium/memory/item.hpp>

#include <cassert>
#include <cstdlib>
#include <limits>
#include <ostream>
#include <vector>

#ifdef OSMIUM_ITEM_STORAGE_GC_DEBUG
# include <iostream>
# include <chrono>
#endif

namespace osmium {

    /**
     * Class for storing OSM data in memory. Any osmium::memory::Item can be
     * added to the stash and it will be copied into its internal Buffer. To
     * access the item again, an opaque handle is used.
     */
    class ItemStash {

    public:

        /**
         * This is the type of the handle returned by the add_item() call.
         * It is used to access the item again with get_item() or get<>()
         * or erase it with remove_item().
         *
         * There is one special handle, the invalid handle. It can be created
         * by calling the default constructor. You can use it for instance to
         * mark removed objects in your data structures. Valid handles can
         * only be constructed by the ItemStash class.
         */
        class handle_type {

            friend class ItemStash;

            std::size_t value; // NOLINT(modernize-use-default-member-init)
                               // Some compilers don't like the default member
                               // init: "error: defaulted default constructor
                               // of 'handle_type' cannot be used by non-static
                               // data member initializer which appears before
                               // end of class definition"

            explicit handle_type(std::size_t new_value) noexcept :
                value(new_value) {
                assert(new_value > 0);
            }

        public:

            /// The default constructor creates an invalid handle.
            handle_type() noexcept :
                value(0) {
            }

            /// Is this a valid handle?
            bool valid() const noexcept {
                return value != 0;
            }

            /**
             * Print the handle for debugging purposes. An invalid handle
             * will be printed as the single letter '-'. A valid handle will
             * be printed as a unique (for an ItemStash object) number.
             */
            template <typename TChar, typename TTraits>
            friend inline std::basic_ostream<TChar, TTraits>& operator<<(std::basic_ostream<TChar, TTraits>& out, const ItemStash::handle_type& handle) {
                if (handle.valid()) {
                    out << handle.value;
                } else {
                    out << '-';
                }
                return out;
            }

        }; // class handle_type

    private:

        enum {
            initial_buffer_size = 1024UL * 1024UL
        };

        enum {
            removed_item_offset = std::numeric_limits<std::size_t>::max()
        };

        osmium::memory::Buffer m_buffer;
        std::vector<std::size_t> m_index;
        std::size_t m_count_items = 0;
        std::size_t m_count_removed = 0;
#ifdef OSMIUM_ITEM_STORAGE_GC_DEBUG
        int64_t m_gc_time = 0;
#endif

        class cleanup_helper {

            std::vector<std::size_t>& m_index;
            std::size_t m_pos = 0;

        public:

            explicit cleanup_helper(std::vector<std::size_t>& index) :
                m_index(index) {
            }

            void moving_in_buffer(std::size_t old_offset, std::size_t new_offset) {
                while (m_index[m_pos] != old_offset) {
                    ++m_pos;
                    assert(m_pos < m_index.size());
                }
                m_index[m_pos] = new_offset;
                ++m_pos;
            }

        }; // cleanup_helper

        std::size_t& get_item_offset_ref(handle_type handle) noexcept {
            assert(handle.valid() && "handle must be valid");
            assert(handle.value <= m_index.size());
            auto& offset = m_index[handle.value - 1];
            assert(offset != removed_item_offset);
            assert(offset < m_buffer.committed());
            return offset;
        }

        std::size_t get_item_offset(handle_type handle) const noexcept {
            assert(handle.valid() && "handle must be valid");
            assert(handle.value <= m_index.size());
            const auto& offset = m_index[handle.value - 1];
            assert(offset != removed_item_offset);
            assert(offset < m_buffer.committed());
            return offset;
        }

        // This function decides whether it makes sense to garbage collect the
        // database. The values here are the result of some experimentation
        // with real data. We need to balance the memory use with the time
        // spent on garbage collecting. We don't need to garbage collect if
        // there is enough space in the buffer anyway (*4). On the other hand,
        // if there aren't enough removed objects we would just call the
        // garbage collection again and again, then it is better to let the
        // buffer grow (*3). The checks (*1) and (*2) make sure there is
        // minimum and maximum for the number of removed objects.
        bool should_gc() const noexcept {
            if (m_count_removed < 10 * 1000) { // *1
                return false;
            }
            if (m_count_removed >  5 * 1000 * 1000) { // *2
                return true;
            }
            if (m_count_removed * 5 < m_count_items) { // *3
                return false;
            }
            return m_buffer.capacity() - m_buffer.committed() < 10 * 1024; // *4
        }

    public:

        ItemStash() :
            m_buffer(initial_buffer_size, osmium::memory::Buffer::auto_grow::yes) {
        }

        /**
         * Return an estimate of the number of bytes currently used by this
         * ItemStash instance.
         *
         * Complexity: Constant.
         */
        std::size_t used_memory() const noexcept {
            return sizeof(ItemStash) +
                   m_buffer.capacity() +
                   m_index.capacity() * sizeof(std::size_t);
        }

        /**
         * The number of items currently in the stash. This is the number
         * added minus the number removed.
         *
         * Complexity: Constant.
         */
        std::size_t size() const noexcept {
            return m_count_items;
        }

        /**
         * The number of removed items currently still taking up memory in
         * the stash. You can call garbage_collect() to remove them.
         *
         * Complexity: Constant.
         */
        std::size_t count_removed() const noexcept {
            return m_count_removed;
        }

        /**
         * Clear all items from the stash. This will not necessarily release
         * any memory. All handles are invalidated.
         */
        void clear() {
            m_buffer.clear();
            m_index.clear();
            m_count_items = 0;
            m_count_removed = 0;
        }

        /**
         * Add an item to the stash. This will invalidate any pointers and
         * references into the stash, but handles are still valid.
         *
         * Complexity: Amortized constant.
         */
        handle_type add_item(const osmium::memory::Item& item) {
            if (should_gc()) {
                garbage_collect();
            }
            ++m_count_items;
            const auto offset = m_buffer.committed();
            m_buffer.add_item(item);
            m_buffer.commit();
            m_index.push_back(offset);
            return handle_type{m_index.size()};
        }

        /**
         * Get a reference to an item in the stash. Note that this reference
         * will be invalidated by any add_item() or clear() calls.
         *
         * Complexity: Constant.
         *
         * @param handle A handle returned by add_item().
         *
         * @pre Handle must be a valid handle and referring to a non-removed
         *      item.
         */
        osmium::memory::Item& get_item(handle_type handle) const {
            return m_buffer.get<osmium::memory::Item>(get_item_offset(handle));
        }

        /**
         * Get a reference to an item in the stash. Note that this reference
         * will be invalidated by any add_item() or clear() calls.
         *
         * Complexity: Constant.
         *
         * @param handle A handle returned by add_item().
         * @tparam T Type you want to the data to be interpreted as. You must
         *         be sure that the item has the specified type, this will
         *         not be checked!
         * @returns Reference of given type pointing to the data in the
         *          stash.
         * @pre Handle must be a valid handle and referring to a non-removed
         *      item.
         */
        template <typename T>
        T& get(handle_type handle) const {
            return static_cast<T&>(get_item(handle));
        }

        /**
         * Garbage collect the memory used by the ItemStash. This will free up
         * memory for adding new items. No memory is actually returned to the
         * OS. Usually you do not need to call this, because add_item() will
         * call it for you as necessary.
         *
         * Complexity: Linear in size() + count_removed().
         */
        void garbage_collect() {
#ifdef OSMIUM_ITEM_STORAGE_GC_DEBUG
            std::cerr << "GC items=" << m_count_items << " removed=" << m_count_removed << " buffer.committed=" << m_buffer.committed() << " buffer.capacity=" << m_buffer.capacity() << "\n";
            using clock = std::chrono::high_resolution_clock;
            std::chrono::time_point<clock> start = clock::now();
#endif

            m_count_removed = 0;
            cleanup_helper helper{m_index};
            m_buffer.purge_removed(&helper);

#ifdef OSMIUM_ITEM_STORAGE_GC_DEBUG
            std::chrono::time_point<clock> stop = clock::now();
            const int64_t time = std::chrono::duration_cast<std::chrono::microseconds>(stop - start).count();
            m_gc_time += time;
            std::cerr << "        time=" << time
                      << "us total=" << m_gc_time << "us\n";
#endif
        }

        /**
         * Remove an item from the stash. The item will be marked as removed
         * and the handle will be invalidated. No memory will actually be
         * freed.
         *
         * Complexity: Constant.
         *
         * @param handle A handle returned by add_item().
         *
         * @pre Handle must be a valid handle and referring to a non-removed
         *      item.
         */
        void remove_item(handle_type handle) {
            auto& offset = get_item_offset_ref(handle);
            auto& item = m_buffer.get<osmium::memory::Item>(offset);
            assert(!item.removed() && "can not call remove_item() on already removed item");
            item.set_removed(true);
            offset = removed_item_offset;
            --m_count_items;
            ++m_count_removed;
        }

    }; // class ItemStash

} // namespace osmium

#endif // OSMIUM_STORAGE_ITEM_STASH_HPP
