#ifndef OSMIUM_GEOM_UTIL_HPP
#define OSMIUM_GEOM_UTIL_HPP

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

#include <osmium/util/compatibility.hpp>

#include <stdexcept>
#include <string>

namespace osmium {

    /**
     * Exception thrown when a projection object can not be initialized or the
     * projection of some coordinates can not be calculated.
     */
    struct OSMIUM_EXPORT projection_error : public std::runtime_error {

        explicit projection_error(const std::string& what) :
            std::runtime_error(what) {
        }

        explicit projection_error(const char* what) :
            std::runtime_error(what) {
        }

    }; // struct projection_error

    namespace geom {

        constexpr double PI = 3.14159265358979323846;

        /// Convert angle from degrees to radians.
        constexpr double deg_to_rad(double degree) noexcept {
            return degree * (PI / 180.0);
        }

        /// Convert angle from radians to degrees.
        constexpr double rad_to_deg(double radians) noexcept {
            return radians * (180.0 / PI);
        }

    } // namespace geom

} // namespace osmium

#endif // OSMIUM_GEOM_UTIL_HPP
