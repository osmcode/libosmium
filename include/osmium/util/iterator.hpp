#ifndef OSMIUM_UTIL_ITERATOR_HPP
#define OSMIUM_UTIL_ITERATOR_HPP

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

#include <type_traits>
#include <utility>

namespace osmium {

    template <typename It, typename P = std::pair<It, It>>
    struct iterator_range : public P {

        using iterator = It;

        explicit iterator_range(P&& p) noexcept :
            P(std::move(p)) {
        }

        It begin() const noexcept {
            return this->first;
        }

        It end() const noexcept {
            return this->second;
        }

        bool empty() const noexcept {
            return begin() == end();
        }

    }; // struct iterator_range

    /**
     * Helper function to create iterator_range from std::pair.
     */
    template <typename P, typename It = typename P::first_type>
    inline iterator_range<It> make_range(P&& p) noexcept {
        static_assert(std::is_same<P, std::pair<It, It>>::value, "make_range needs pair of iterators as argument");
        return iterator_range<It>{std::forward<P>(p)};
    }

} // namespace osmium

#endif // OSMIUM_UTIL_ITERATOR_HPP
