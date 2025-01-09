#ifndef OSMIUM_AREA_MULTIPOLYGON_MANAGER_LEGACY_HPP
#define OSMIUM_AREA_MULTIPOLYGON_MANAGER_LEGACY_HPP

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

#include <osmium/area/stats.hpp>
#include <osmium/handler.hpp>
#include <osmium/handler/check_order.hpp>
#include <osmium/memory/buffer.hpp>
#include <osmium/memory/callback_buffer.hpp>
#include <osmium/osm/item_type.hpp>
#include <osmium/osm/relation.hpp>
#include <osmium/osm/tag.hpp>
#include <osmium/osm/way.hpp>
#include <osmium/relations/manager_util.hpp>
#include <osmium/relations/members_database.hpp>
#include <osmium/relations/relations_database.hpp>
#include <osmium/relations/relations_manager.hpp>
#include <osmium/storage/item_stash.hpp>

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <vector>

namespace osmium {

    /**
     * @brief Code related to the building of areas (multipolygons) from relations.
     */
    namespace area {

        /**
         * This class collects all data needed for creating areas from
         * relations tagged with type=multipolygon or type=boundary.
         * Most of its functionality is derived from the parent class
         * osmium::relations::Collector.
         *
         * The actual assembling of the areas is done by the assembler
         * class given as template argument.
         *
         * @tparam TAssembler Multipolygon Assembler class.
         * @pre The Ids of all objects must be unique in the input data.
         */
        template <typename TAssembler>
        class MultipolygonManagerLegacy : public osmium::relations::RelationsManager<MultipolygonManagerLegacy<TAssembler>, false, true, false> {

            using assembler_config_type = typename TAssembler::config_type;
            assembler_config_type m_assembler_config;

            area_stats m_stats;

        public:

            /**
             * Construct a MultipolygonManagerLegacy.
             *
             * @param assembler_config The configuration that will be given to
             *                         any newly constructed area assembler.
             */
            explicit MultipolygonManagerLegacy(const assembler_config_type& assembler_config) :
                m_assembler_config(assembler_config) {
            }

            /**
             * Access the aggregated statistics generated by the assemblers
             * called from the manager.
             */
            const area_stats& stats() const noexcept {
                return m_stats;
            }

            /**
             * We are interested in all relations tagged with type=multipolygon
             * or type=boundary with at least one way member.
             */
            bool new_relation(const osmium::Relation& relation) const {
                const char* type = relation.tags().get_value_by_key("type");

                // ignore relations without "type" tag
                if (!type) {
                    return false;
                }

                if ((!std::strcmp(type, "multipolygon")) || (!std::strcmp(type, "boundary"))) {
                    return std::any_of(relation.members().cbegin(), relation.members().cend(), [](const RelationMember& member) {
                        return member.type() == osmium::item_type::way;
                    });
                }

                return false;
            }

            /**
             * This is called when a relation is complete, ie. all members
             * were found in the input. It will build the area using the
             * assembler.
             */
            void complete_relation(const osmium::Relation& relation) {
                std::vector<const osmium::Way*> ways;
                ways.reserve(relation.members().size());
                for (const auto& member : relation.members()) {
                    if (member.ref() != 0) {
                        ways.push_back(this->get_member_way(member.ref()));
                        assert(ways.back() != nullptr);
                    }
                }

                try {
                    TAssembler assembler{m_assembler_config};
                    assembler(relation, ways, this->buffer());
                    m_stats += assembler.stats();
                } catch (const osmium::invalid_location&) {
                    // XXX ignore
                }
            }

            /**
             * This is called when a way is not in any multipolygon
             * relation.
             */
            void way_not_in_any_relation(const osmium::Way& way) {
                // you need at least 4 nodes to make up a polygon
                if (way.nodes().size() <= 3) {
                    return;
                }
                try {
                    if (!way.nodes().front().location() || !way.nodes().back().location()) {
                        throw osmium::invalid_location{"invalid location"};
                    }
                    if (way.ends_have_same_location()) {
                        // way is closed and has enough nodes, build simple multipolygon
                        TAssembler assembler{m_assembler_config};
                        assembler(way, this->buffer());
                        m_stats += assembler.stats();
                    }
                } catch (const osmium::invalid_location&) {
                    // XXX ignore
                }
            }

        }; // class MultipolygonManagerLegacy

    } // namespace area

} // namespace osmium

#endif // OSMIUM_AREA_MULTIPOLYGON_MANAGER_LEGACY_HPP
