#ifdef STAND_ALONE
# define BOOST_TEST_MODULE Main
#endif
#include <boost/test/unit_test.hpp>

#include <osmium/builder/osm_object_builder.hpp>
#include <osmium/osm/way.hpp>

#include "helper.hpp"

BOOST_AUTO_TEST_SUITE(Basic_Way)

BOOST_AUTO_TEST_CASE(way_builder) {
    osmium::memory::Buffer buffer(10000);

    osmium::Way& way = buffer_add_way(buffer,
        "foo",
        {{"highway", "residential"}, {"name", "High Street"}},
        {1, 3, 2});

    way.id(17)
        .version(3)
        .visible(true)
        .changeset(333)
        .uid(21)
        .timestamp(123);

    BOOST_CHECK_EQUAL(17, way.id());
    BOOST_CHECK_EQUAL(3, way.version());
    BOOST_CHECK_EQUAL(true, way.visible());
    BOOST_CHECK_EQUAL(333, way.changeset());
    BOOST_CHECK_EQUAL(21, way.uid());
    BOOST_CHECK_EQUAL(std::string("foo"), way.user());
    BOOST_CHECK_EQUAL(123, way.timestamp());
    BOOST_CHECK_EQUAL(2, way.tags().size());
    BOOST_CHECK_EQUAL(3, way.nodes().size());
    BOOST_CHECK_EQUAL(1, way.nodes()[0].ref());
    BOOST_CHECK_EQUAL(3, way.nodes()[1].ref());
    BOOST_CHECK_EQUAL(2, way.nodes()[2].ref());
    BOOST_CHECK(! way.is_closed());
}

BOOST_AUTO_TEST_CASE(closed_way) {
    osmium::memory::Buffer buffer(10000);

    osmium::Way& way = buffer_add_way(buffer,
        "foo",
        {{"highway", "residential"}, {"name", "High Street"}},
        {1, 3, 1});

    BOOST_CHECK(way.is_closed());
}

BOOST_AUTO_TEST_CASE(way_builder_with_helpers) {
    osmium::memory::Buffer buffer(10000);
    {
        osmium::builder::WayBuilder builder(buffer);
        builder.add_user("username");
        builder.add_tags({
            {"amenity", "restaurant"},
            {"name", "Zum goldenen Schwanen"}
        });
        builder.add_node_refs({
            {22, {3.5, 4.7}},
            {67, {4.1, 2.2}}
        });
    }
    buffer.commit();
    osmium::Way& way = buffer.get<osmium::Way>(0);

    BOOST_CHECK_EQUAL(std::string("username"), way.user());

    BOOST_CHECK_EQUAL(2, way.tags().size());
    BOOST_CHECK_EQUAL(std::string("amenity"), way.tags().begin()->key());
    BOOST_CHECK_EQUAL(std::string("Zum goldenen Schwanen"), way.tags()["name"]);

    BOOST_CHECK_EQUAL(2, way.nodes().size());
    BOOST_CHECK_EQUAL(22, way.nodes()[0].ref());
    BOOST_CHECK_EQUAL(4.1, way.nodes()[1].location().lon());
}

BOOST_AUTO_TEST_SUITE_END()
