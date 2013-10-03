#ifdef STAND_ALONE
# define BOOST_TEST_MODULE Main
#endif
#include <boost/test/unit_test.hpp>

#include <osmium/osm/node.hpp>
#include <osmium/osm/ostream.hpp>

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
    BOOST_CHECK_EQUAL(3, node.version());
    BOOST_CHECK_EQUAL(true, node.visible());
    BOOST_CHECK_EQUAL(333, node.changeset());
    BOOST_CHECK_EQUAL(21, node.uid());
    BOOST_CHECK(!strcmp("foo", node.user()));
    BOOST_CHECK_EQUAL(123, node.timestamp());
    BOOST_CHECK_EQUAL(osmium::Location(3.5, 4.7), node.location());
}

BOOST_AUTO_TEST_SUITE_END()
