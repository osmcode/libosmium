#include "catch.hpp"
#include "utils.hpp"

#include <osmium/io/xml_input.hpp>
#include <osmium/osm/relation.hpp>
#include <osmium/relations/relations_manager.hpp>

struct EmptyRM : public osmium::relations::RelationsManager<EmptyRM, true, true, true> {
};

struct TestRM : public osmium::relations::RelationsManager<TestRM, true, true, true> {

    std::size_t count_new_rels      = 0;
    std::size_t count_new_members   = 0;
    std::size_t count_complete_rels = 0;
    std::size_t count_before        = 0;
    std::size_t count_not_in_any    = 0;
    std::size_t count_after         = 0;

    bool new_relation(const osmium::Relation& /*relation*/) noexcept {
        ++count_new_rels;
        return true;
    }

    bool new_member(const osmium::Relation& /*relation*/, const osmium::RelationMember& /*member*/, std::size_t /*n*/) noexcept {
        ++count_new_members;
        return true;
    }

    void complete_relation(const osmium::Relation& /*relation*/) noexcept {
        ++count_complete_rels;
    }

    void before_node(const osmium::Node& /*node*/) noexcept {
        ++count_before;
    }

    void node_not_in_any_relation(const osmium::Node& /*node*/) noexcept {
        ++count_not_in_any;
    }

    void after_node(const osmium::Node& /*node*/) noexcept {
        ++count_after;
    }

    void before_way(const osmium::Way& /*way*/) noexcept {
        ++count_before;
    }

    void way_not_in_any_relation(const osmium::Way& /*way*/) noexcept {
        ++count_not_in_any;
    }

    void after_way(const osmium::Way& /*way*/) noexcept {
        ++count_after;
    }

    void before_relation(const osmium::Relation& /*relation*/) noexcept {
        ++count_before;
    }

    void relation_not_in_any_relation(const osmium::Relation& /*relation*/) noexcept {
        ++count_not_in_any;
    }

    void after_relation(const osmium::Relation& /*relation*/) noexcept {
        ++count_after;
    }

};

struct CallbackRM : public osmium::relations::RelationsManager<CallbackRM, true, false, false> {

    std::size_t count_nodes = 0;

    bool new_relation(const osmium::Relation& /*relation*/) noexcept {
        return true;
    }

    bool new_member(const osmium::Relation& /*relation*/, const osmium::RelationMember& member, std::size_t /*n*/) noexcept {
        return member.type() == osmium::item_type::node;
    }

    void complete_relation(const osmium::Relation& relation) noexcept {
        for (const auto& member : relation.members()) {
            if (member.type() == osmium::item_type::node) {
                ++count_nodes;
                const auto& node = member_nodes_database().get(member.ref());
                buffer().add_item(node);
                buffer().commit();
            }
        }
    }

};

TEST_CASE("Use RelationsManager without any overloaded functions in derived class") {
    osmium::io::File file{with_data_dir("t/relations/data.osm")};

    EmptyRM manager;

    osmium::relations::read_relations(file, manager);

    REQUIRE(manager.member_nodes_database().size()     == 2);
    REQUIRE(manager.member_ways_database().size()      == 2);
    REQUIRE(manager.member_relations_database().size() == 1);

    REQUIRE(manager.member_database(osmium::item_type::node).size()     == 2);
    REQUIRE(manager.member_database(osmium::item_type::way).size()      == 2);
    REQUIRE(manager.member_database(osmium::item_type::relation).size() == 1);

    const auto& m = manager;
    REQUIRE(m.member_database(osmium::item_type::node).size()     == 2);
    REQUIRE(m.member_database(osmium::item_type::way).size()      == 2);
    REQUIRE(m.member_database(osmium::item_type::relation).size() == 1);

    osmium::io::Reader reader{file};
    osmium::apply(reader, manager.handler());
    reader.close();
}

TEST_CASE("Relations manager derived class") {
    osmium::io::File file{with_data_dir("t/relations/data.osm")};

    TestRM manager;

    osmium::relations::read_relations(file, manager);

    REQUIRE(manager.member_nodes_database().size()     == 2);
    REQUIRE(manager.member_ways_database().size()      == 2);
    REQUIRE(manager.member_relations_database().size() == 1);

    bool callback_called = false;
    osmium::io::Reader reader{file};
    osmium::apply(reader, manager.handler([&](osmium::memory::Buffer&&) {
        callback_called = true;
    }));
    reader.close();
    REQUIRE_FALSE(callback_called);

    REQUIRE(manager.count_new_rels      ==  3);
    REQUIRE(manager.count_new_members   ==  5);
    REQUIRE(manager.count_complete_rels ==  2);
    REQUIRE(manager.count_before        == 10);
    REQUIRE(manager.count_not_in_any    ==  6);
    REQUIRE(manager.count_after         == 10);

    int n = 0;
    manager.for_each_incomplete_relation([&](const osmium::relations::RelationHandle&){
        ++n;
    });
    REQUIRE(n == 1);
}

TEST_CASE("Relations manager with callback") {
    osmium::io::File file{with_data_dir("t/relations/data.osm")};

    CallbackRM manager;

    osmium::relations::read_relations(file, manager);

    REQUIRE(manager.member_nodes_database().size()     == 2);
    REQUIRE(manager.member_ways_database().size()      == 0);
    REQUIRE(manager.member_relations_database().size() == 0);

    bool callback_called = false;
    osmium::io::Reader reader{file};
    osmium::apply(reader, manager.handler([&](osmium::memory::Buffer&& buffer) {
        callback_called = true;
        REQUIRE(std::distance(buffer.begin(), buffer.end()) == 2);
    }));
    reader.close();
    REQUIRE(manager.count_nodes == 2);
    REQUIRE(callback_called);
}

