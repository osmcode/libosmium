#ifndef TEST_BASIC_HELPER_HPP
#define TEST_BASIC_HELPER_HPP

#include <utility>
#include <vector>

#include <osmium/builder/osm_object_builder.hpp>

inline void add_tags(osmium::memory::Buffer& buffer, osmium::builder::Builder& builder, const std::vector<std::pair<const char*, const char*>>& tags) {
    osmium::builder::TagListBuilder tl_builder(buffer, &builder);
    for (const auto& tag : tags) {
        tl_builder.add_tag(tag.first, tag.second);
    }
}

inline osmium::Area& buffer_add_area(osmium::memory::Buffer& buffer, const char* user,
        const std::vector<std::pair<const char*, const char*>>& tags,
        const std::vector<std::pair<bool,
            std::vector<std::pair<osmium::object_id_type, osmium::Location>>>>& rings) {
    osmium::builder::AreaBuilder builder(buffer);
    builder.add_user(user);
    add_tags(buffer, builder, tags);

    for (const auto& ring : rings) {
        if (ring.first) {
            osmium::builder::OuterRingBuilder ring_builder(buffer, &builder);
            for (const auto& p : ring.second) {
                ring_builder.add_node_ref(p.first, p.second);
            }
        } else {
            osmium::builder::InnerRingBuilder ring_builder(buffer, &builder);
            for (const auto& p : ring.second) {
                ring_builder.add_node_ref(p.first, p.second);
            }
        }
    }
    buffer.commit();
    return builder.object();
}

#endif // TEST_BASIC_HELPER_HPP
