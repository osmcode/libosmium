#ifndef OSMIUM_AREA_PROBLEM
#define OSMIUM_AREA_PROBLEM

/*

This file is part of Osmium (http://osmcode.org/osmium).

Copyright 2013,2014 Jochen Topf <jochen@topf.org> and others (see README).

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

#include <osmium/osm/noderef.hpp>
#include <osmium/osm/ostream.hpp>
#include <osmium/area/segment.hpp>

namespace osmium {

    namespace area {

        class Problem {

        public:

            enum class problem_type : int {
                intersection    = 0,
                ring_not_closed = 1
            }; // enum class problem_type

        public:

            problem_type m_problem;
            NodeRef m_node_ref;
            NodeRefSegment m_segment1;
            NodeRefSegment m_segment2;

            Problem(problem_type problem, const osmium::NodeRef& node_ref = osmium::NodeRef(), const NodeRefSegment& segment1 = NodeRefSegment(), const NodeRefSegment& segment2 = NodeRefSegment()) :
                m_problem(problem),
                m_node_ref(node_ref),
                m_segment1(segment1),
                m_segment2(segment2) {
            }

            std::string problem() const {
                std::string p;
                switch (m_problem) {
                    case problem_type::intersection:
                        p = "intersection";
                        break;
                    case problem_type::ring_not_closed:
                        p = "ring_not_closed";
                        break;
                }
                return p;
            }

        }; // class Problem

    } // namespace area

} // namespace osmium

#endif // OSMIUM_AREA_PROBLEM
