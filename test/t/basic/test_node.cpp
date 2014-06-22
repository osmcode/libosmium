#ifdef STAND_ALONE
# define BOOST_TEST_MODULE Main
#endif
#include <boost/test/unit_test.hpp>

#include <osmium/osm/node.hpp>

#include "helper.hpp"

BOOST_AUTO_TEST_SUITE(Basic_Node)

BOOST_AUTO_TEST_CASE(node_builder) {
    osmium::memory::Buffer buffer(10000);

    osmium::Node& node = buffer_add_node(buffer,
        "foo",
        {{"amenity", "pub"}, {"name", "OSM BAR"}},
        {3.5, 4.7});

    node.id(17)
        .version(3)
        .visible(true)
        .changeset(333)
        .uid(21)
        .timestamp(123);

    BOOST_CHECK_EQUAL(17, node.id());
    BOOST_CHECK_EQUAL(17, node.positive_id());
    BOOST_CHECK_EQUAL(3, node.version());
    BOOST_CHECK_EQUAL(true, node.visible());
    BOOST_CHECK_EQUAL(false, node.deleted());
    BOOST_CHECK_EQUAL(333, node.changeset());
    BOOST_CHECK_EQUAL(21, node.uid());
    BOOST_CHECK_EQUAL(std::string("foo"), node.user());
    BOOST_CHECK_EQUAL(123, node.timestamp());
    BOOST_CHECK_EQUAL(osmium::Location(3.5, 4.7), node.location());
    BOOST_CHECK_EQUAL(2, node.tags().size());

    node.visible(false);
    BOOST_CHECK_EQUAL(false, node.visible());
    BOOST_CHECK_EQUAL(true, node.deleted());
}

BOOST_AUTO_TEST_CASE(node_default_attributes) {
    osmium::memory::Buffer buffer(10000);

    osmium::Node& node = buffer_add_node(buffer, "", {}, osmium::Location{});

    BOOST_CHECK_EQUAL(0, node.id());
    BOOST_CHECK_EQUAL(0, node.positive_id());
    BOOST_CHECK_EQUAL(0, node.version());
    BOOST_CHECK_EQUAL(true, node.visible());
    BOOST_CHECK_EQUAL(0, node.changeset());
    BOOST_CHECK_EQUAL(0, node.uid());
    BOOST_CHECK_EQUAL(std::string(""), node.user());
    BOOST_CHECK_EQUAL(0, node.timestamp());
    BOOST_CHECK_EQUAL(osmium::Location(), node.location());
    BOOST_CHECK_EQUAL(0, node.tags().size());
}

BOOST_AUTO_TEST_CASE(set_node_attributes_from_string) {
    osmium::memory::Buffer buffer(10000);

    osmium::Node& node = buffer_add_node(buffer,
        "foo",
        {{"amenity", "pub"}, {"name", "OSM BAR"}},
        {3.5, 4.7});

    node.id("-17")
        .version("3")
        .visible(true)
        .changeset("333")
        .uid("21");

    BOOST_CHECK_EQUAL(-17, node.id());
    BOOST_CHECK_EQUAL(17, node.positive_id());
    BOOST_CHECK_EQUAL(3, node.version());
    BOOST_CHECK_EQUAL(true, node.visible());
    BOOST_CHECK_EQUAL(333, node.changeset());
    BOOST_CHECK_EQUAL(21, node.uid());
}

BOOST_AUTO_TEST_CASE(large_id) {
    osmium::memory::Buffer buffer(10000);

    osmium::Node& node = buffer_add_node(buffer, "", {}, osmium::Location{});

    int64_t id = 3000000000l;
    node.id(id);

    BOOST_CHECK_EQUAL(id, node.id());
    BOOST_CHECK_EQUAL(id, node.positive_id());

    node.id(-id);
    BOOST_CHECK_EQUAL(-id, node.id());
    BOOST_CHECK_EQUAL(id, node.positive_id());
}

BOOST_AUTO_TEST_CASE(tags) {
    osmium::memory::Buffer buffer(10000);

    osmium::Node& node = buffer_add_node(buffer,
        "foo",
        {{"amenity", "pub"}, {"name", "OSM BAR"}},
        {3.5, 4.7});

    BOOST_CHECK(nullptr == node.tags().get_value_by_key("fail"));
    BOOST_CHECK_EQUAL(std::string("pub"), node.tags().get_value_by_key("amenity"));

    BOOST_CHECK_EQUAL(std::string("default"), node.tags().get_value_by_key("fail", "default"));
    BOOST_CHECK_EQUAL(std::string("pub"), node.tags().get_value_by_key("amenity", "default"));
}


BOOST_AUTO_TEST_SUITE_END()
