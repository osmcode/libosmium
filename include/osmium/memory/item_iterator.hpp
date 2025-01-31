#ifndef OSMIUM_MEMORY_ITEM_ITERATOR_HPP
#define OSMIUM_MEMORY_ITEM_ITERATOR_HPP

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

#include <osmium/memory/item.hpp>
#include <osmium/osm/item_type.hpp>

#include <cassert>
#include <cstddef>
#include <iosfwd>
#include <iterator>
#include <type_traits>

namespace osmium {

    namespace memory {

        namespace detail {

            template <typename T>
            constexpr bool type_is_compatible(const osmium::item_type t) noexcept {
                return T::is_compatible_to(t);
            }

        } // namespace detail

        template <typename TMember>
        class ItemIterator {

            static_assert(std::is_base_of<osmium::memory::Item, TMember>::value, "TMember must derive from osmium::memory::Item");

            // This data_type is either 'unsigned char*' or 'const unsigned char*' depending
            // on whether TMember is const. This allows this class to be used as an iterator and
            // as a const_iterator.
            using data_type = std::conditional_t<std::is_const<TMember>::value, const unsigned char*, unsigned char*>;

            data_type m_data;
            data_type m_end;

            void advance_to_next_item_of_right_type() noexcept {
                while (m_data != m_end &&
                       !detail::type_is_compatible<TMember>(reinterpret_cast<const osmium::memory::Item*>(m_data)->type())) {
                    m_data = reinterpret_cast<TMember*>(m_data)->next();
                }
            }

        public:

            using iterator_category = std::forward_iterator_tag;
            using value_type        = TMember;
            using difference_type   = std::ptrdiff_t;
            using pointer           = value_type*;
            using reference         = value_type&;

            ItemIterator() noexcept :
                m_data(nullptr),
                m_end(nullptr) {
            }

            ItemIterator(data_type data, data_type end) noexcept :
                m_data(data),
                m_end(end) {
                advance_to_next_item_of_right_type();
            }

            template <typename T>
            ItemIterator<T> cast() const noexcept {
                return ItemIterator<T>(m_data, m_end);
            }

            ItemIterator<TMember>& operator++() noexcept {
                assert(m_data);
                assert(m_data != m_end);
                m_data = reinterpret_cast<TMember*>(m_data)->next();
                advance_to_next_item_of_right_type();
                return *static_cast<ItemIterator<TMember>*>(this);
            }

            /**
             * Like operator++() but will NOT skip items of unwanted
             * types. Do not use this unless you know what you are
             * doing.
             */
            ItemIterator<TMember>& advance_once() noexcept {
                assert(m_data);
                assert(m_data != m_end);
                m_data = reinterpret_cast<TMember*>(m_data)->next();
                return *static_cast<ItemIterator<TMember>*>(this);
            }

            ItemIterator<TMember> operator++(int) noexcept {
                ItemIterator<TMember> tmp{*this};
                operator++();
                return tmp;
            }

            bool operator==(const ItemIterator<TMember>& rhs) const noexcept {
                return m_data == rhs.m_data && m_end == rhs.m_end;
            }

            bool operator!=(const ItemIterator<TMember>& rhs) const noexcept {
                return !(*this == rhs);
            }

            data_type data() noexcept {
                assert(m_data);
                return m_data;
            }

            const unsigned char* data() const noexcept {
                assert(m_data);
                return m_data;
            }

            TMember& operator*() const noexcept {
                assert(m_data);
                assert(m_data != m_end);
                return *reinterpret_cast<TMember*>(m_data);
            }

            TMember* operator->() const noexcept {
                assert(m_data);
                assert(m_data != m_end);
                return reinterpret_cast<TMember*>(m_data);
            }

            explicit operator bool() const noexcept {
                return (m_data != nullptr) && (m_data != m_end);
            }

            template <typename TChar, typename TTraits>
            void print(std::basic_ostream<TChar, TTraits>& out) const {
                out << static_cast<const void*>(m_data);
            }

        }; // class ItemIterator

        template <typename TChar, typename TTraits, typename TMember>
        inline std::basic_ostream<TChar, TTraits>& operator<<(std::basic_ostream<TChar, TTraits>& out, const ItemIterator<TMember>& iter) {
            iter.print(out);
            return out;
        }

        template <typename T>
        class ItemIteratorRange {

            static_assert(std::is_base_of<osmium::memory::Item, T>::value, "Template parameter must derive from osmium::memory::Item");

            // This data_type is either 'unsigned char*' or
            // 'const unsigned char*' depending on whether T is const.
            using data_type = std::conditional_t<std::is_const<T>::value, const unsigned char*, unsigned char*>;

            data_type m_begin;
            data_type m_end;

        public:

            using iterator = ItemIterator<T>;
            using const_iterator = ItemIterator<const T>;

            ItemIteratorRange(data_type first, data_type last) noexcept :
                m_begin(first),
                m_end(last) {
            }

            iterator begin() noexcept {
                return iterator{m_begin, m_end};
            }

            iterator end() noexcept {
                return iterator{m_end, m_end};
            }

            const_iterator cbegin() const noexcept {
                return const_iterator{m_begin, m_end};
            }

            const_iterator cend() const noexcept {
                return const_iterator{m_end, m_end};
            }

            const_iterator begin() const noexcept {
                return cbegin();
            }

            const_iterator end() const noexcept {
                return cend();
            }

            /**
             * Return the number of items in this range.
             *
             * Complexity: Linear in the number of items.
             */
            std::size_t size() const noexcept {
                if (m_begin == m_end) {
                    return 0;
                }
                return std::distance(cbegin(), cend());
            }

            /**
             * Is this range empty?
             *
             * Complexity: Linear in the number of items.
             */
            bool empty() const noexcept {
                return size() == 0;
            }

        }; // class ItemIteratorRange

    } // namespace memory

} // namespace osmium

#endif // OSMIUM_MEMORY_ITEM_ITERATOR_HPP
