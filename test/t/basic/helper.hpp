#ifndef TEST_HELPER_HPP
#define TEST_HELPER_HPP

#include <tuple>
#include <utility>
#include <vector>

#include <osmium/osm/builder.hpp>

inline void add_tags(osmium::memory::Buffer& buffer, osmium::memory::Builder& builder, const std::vector<std::pair<const char*, const char*>>& tags) {
    osmium::osm::TagListBuilder tl_builder(buffer, &builder);
    for (auto& tag : tags) {
        tl_builder.add_tag(tag.first, tag.second);
    }
}

inline osmium::Node& buffer_add_node(osmium::memory::Buffer& buffer, const char* user, const std::vector<std::pair<const char*, const char*>>& tags, const osmium::Location& location) {
    osmium::osm::NodeBuilder builder(buffer);
    builder.add_user(user);
    add_tags(buffer, builder, tags);
    buffer.commit();
    return builder.object().location(location);
}

inline osmium::Way& buffer_add_way(osmium::memory::Buffer& buffer, const char* user, const std::vector<std::pair<const char*, const char*>>& tags, const std::vector<osmium::object_id_type>& nodes) {
    osmium::osm::WayBuilder builder(buffer);
    builder.add_user(user);
    add_tags(buffer, builder, tags);
    osmium::osm::WayNodeListBuilder wnl_builder(buffer, &builder);
    for (const osmium::object_id_type ref : nodes) {
        wnl_builder.add_way_node(ref);
    }
    buffer.commit();
    return builder.object();
}

inline osmium::Way& buffer_add_way(osmium::memory::Buffer& buffer, const char* user, const std::vector<std::pair<const char*, const char*>>& tags, const std::vector<std::pair<osmium::object_id_type, osmium::Location>>& nodes) {
    osmium::osm::WayBuilder builder(buffer);
    builder.add_user(user);
    add_tags(buffer, builder, tags);
    osmium::osm::WayNodeListBuilder wnl_builder(buffer, &builder);
    for (auto& p : nodes) {
        wnl_builder.add_way_node(p.first, p.second);
    }
    buffer.commit();
    return builder.object();
}

inline osmium::Relation& buffer_add_relation(
        osmium::memory::Buffer& buffer,
        const char* user,
        const std::vector<std::pair<const char*, const char*>>& tags, const std::vector<std::tuple<char, osmium::object_id_type, const char*>>& members) {
    osmium::osm::RelationBuilder builder(buffer);
    builder.add_user(user);
    add_tags(buffer, builder, tags);
    osmium::osm::RelationMemberListBuilder rml_builder(buffer, &builder);
    for (const auto& member : members) {
        rml_builder.add_member(osmium::char_to_item_type(std::get<0>(member)), std::get<1>(member), std::get<2>(member));
    }
    buffer.commit();
    return builder.object();
}

inline osmium::Area& buffer_add_area(osmium::memory::Buffer& buffer, const char* user,
        const std::vector<std::pair<const char*, const char*>>& tags,
        const std::vector<std::pair<bool,
            const std::vector<std::pair<osmium::object_id_type, osmium::Location>>>>& rings) {
    osmium::osm::AreaBuilder builder(buffer);
    builder.add_user(user);
    add_tags(buffer, builder, tags);

    for (auto& ring : rings) {
        if (ring.first) {
            osmium::osm::OuterRingBuilder ring_builder(buffer, &builder);
            for (auto& p : ring.second) {
                ring_builder.add_node_ref(p.first, p.second);
            }
        } else {
            osmium::osm::InnerRingBuilder ring_builder(buffer, &builder);
            for (auto& p : ring.second) {
                ring_builder.add_node_ref(p.first, p.second);
            }
        }
    }
    buffer.commit();
    return builder.object();
}

#endif // TEST_HELPER_HPP
