#ifndef OSMIUM_RELATIONS_DETAIL_RELATION_META_HPP
#define OSMIUM_RELATIONS_DETAIL_RELATION_META_HPP

/*

This file is part of Osmium (http://osmcode.org/libosmium).

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

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <functional>
#include <vector>

namespace osmium {

    namespace relations {

        /**
         * Helper class for the Collector class.
         *
         * Stores information needed to collect all members of a relation. This
         * includes the offset of the relation in a buffer plus the information
         * needed to add members to this relation.
         */
        class RelationMeta {

            /// Vector for relation member offsets
            std::vector<size_t> m_member_offsets;

            /// The relation we are assembling.
            size_t m_relation_offset;

            /**
             * The number of members still needed before the relation is complete.
             * This will be set to the number of members we are interested in and
             * then count down for every member we find. When it is 0, the relation
             * is complete.
             */
            int m_need_members = 0;

        public:

            /**
             * Initialize an empty RelationMeta. This is needed to zero out relations
             * that have been completed.
             */
            RelationMeta() :
                m_member_offsets(),
                m_relation_offset(0) {
            }

            RelationMeta(size_t relation_offset, size_t relation_num_members) :
                m_member_offsets(relation_num_members, -1),
                m_relation_offset(relation_offset) {
            }

            /**
             * Get offset of relation in buffer.
             */
            size_t relation_offset() const {
                return m_relation_offset;
            }

            /**
             * Increment the m_need_members counter.
             */
            void increment_need_members() {
                ++m_need_members;
            }

            /**
             * Add member. This stores the offset to the member object in a
             * buffer. It also decrements the "members needed" counter.
             *
             * @return true if relation is complete, false otherwise
             */
            bool add_member(size_t offset, size_t n) {
                assert(m_need_members > 0);
                assert(n < m_member_offsets.size());
                m_member_offsets[n] = offset;
                return --m_need_members == 0;
            }

            /**
             * Get a vector reference with offsets to all member objects.
             * Note that the pointers can be empty if a member object is of a type
             * we have not requested from the collector (or if it was not in the
             * input).
             */
            std::vector<size_t>& member_offsets() {
                return m_member_offsets;
            }

            const std::vector<size_t>& member_offsets() const {
                return m_member_offsets;
            }

            /**
             * Remove all empty members offsets. These are all the members not
             * found in the input or of the types we are not interested in.
             */
            void remove_empty_members() {
                m_member_offsets.erase(
                    std::remove(m_member_offsets.begin(), m_member_offsets.end(), -1),
                    m_member_offsets.end()
                    );
            }

            /**
             * Returns true if all members for this relation are available.
             */
            bool has_all_members() const {
                return m_need_members == 0;
            }
        
        }; // class RelationMeta

        /**
         * Function object to check if a relation is complete.
         *
         * @return true if this relation is complete, false otherwise.
         */
        struct has_all_members : public std::unary_function<RelationMeta, bool> {

            bool operator()(RelationMeta& relation_info) const {
                return relation_info.has_all_members();
            }

        }; // struct has_all_members

    } // namespace relations

} // namespace osmium

#endif // OSMIUM_RELATIONS_DETAIL_RELATION_META_HPP
