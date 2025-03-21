#ifndef OSMIUM_UTIL_TIMER_HPP
#define OSMIUM_UTIL_TIMER_HPP

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

#include <cstdint>

#ifdef OSMIUM_WITH_TIMER

#include <chrono>

namespace osmium {

    class Timer {

        using clock = std::chrono::high_resolution_clock;
        std::chrono::time_point<clock> m_start;
        std::chrono::time_point<clock> m_stop;

    public:

        Timer() :
            m_start(clock::now()) {
        }

        void start() {
            m_start = clock::now();
        }

        void stop() {
            m_stop = clock::now();
        }

        int64_t elapsed_microseconds() const {
            return std::chrono::duration_cast<std::chrono::microseconds>(m_stop - m_start).count();
        }

    };

} // namespace osmium

#else

namespace osmium {

    class Timer {

    public:

        Timer() = default;

        void start() noexcept { // NOLINT(readability-convert-member-functions-to-static)
        }

        void stop() noexcept { // NOLINT(readability-convert-member-functions-to-static)
        }

        int64_t elapsed_microseconds() const noexcept { // NOLINT(readability-convert-member-functions-to-static)
            return 0;
        }

    };

} // namespace osmium

#endif

#endif // OSMIUM_UTIL_TIMER_HPP
