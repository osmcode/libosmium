#ifndef TEST_HELPER_HPP
#define TEST_HELPER_HPP

#include <tuple>
#include <utility>
#include <vector>

#include <osmium/osm/builder.hpp>

inline void add_tags(osmium::memory::Buffer& buffer, osmium::memory::Builder& builder, const std::vector<std::pair<const char*, const char*>> tags) {
    osmium::memory::TagListBuilder tl_builder(buffer, &builder);
    for (auto& tag : tags) {
        tl_builder.add_tag(tag.first, tag.second);
    }
}

inline osmium::Node& buffer_add_node(osmium::memory::Buffer& buffer, const char* user, const std::vector<std::pair<const char*, const char*>>& tags, const osmium::Location& location) {
    osmium::memory::NodeBuilder builder(buffer);
    builder.add_string(user);
    add_tags(buffer, builder, tags);
    buffer.commit();
    return builder.object().location(location);
}

inline osmium::Way& buffer_add_way(osmium::memory::Buffer& buffer, const char* user, const std::vector<std::pair<const char*, const char*>>& tags, const std::vector<osmium::object_id_type>& nodes) {
    osmium::memory::WayBuilder builder(buffer);
    builder.add_string(user);
    add_tags(buffer, builder, tags);
    osmium::memory::WayNodeListBuilder wnl_builder(buffer, &builder);
    for (const osmium::object_id_type ref : nodes) {
        wnl_builder.add_way_node(ref);
    }
    buffer.commit();
    return builder.object();
}

inline osmium::Relation& buffer_add_relation(
        osmium::memory::Buffer& buffer,
        const char* user,
        const std::vector<std::pair<const char*, const char*>>& tags, const std::vector<std::tuple<char, osmium::object_id_type, const char*>>& members) {
    osmium::memory::RelationBuilder builder(buffer);
    builder.add_string(user);
    add_tags(buffer, builder, tags);
    osmium::memory::RelationMemberListBuilder rml_builder(buffer, &builder);
    for (const auto& member : members) {
        rml_builder.add_member(osmium::char_to_item_type(std::get<0>(member)), std::get<1>(member), std::get<2>(member));
    }
    buffer.commit();
    return builder.object();
}

#endif // TEST_HELPER_HPP
