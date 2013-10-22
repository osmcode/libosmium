#ifdef STAND_ALONE
# define BOOST_TEST_MODULE Main
#endif
#include <boost/test/unit_test.hpp>

#include <boost/regex.hpp>

#include <osmium/geom/wkt.hpp>
#include <osmium/osm/ostream.hpp>

#include "../basic/helper.hpp"

#define OSMIUM_LINK_WITH_LIBS_BOOST_REGEX "-lboost_regex"

BOOST_AUTO_TEST_SUITE(WKT_Geometry)

BOOST_AUTO_TEST_CASE(point) {
    osmium::geom::WKTFactory factory;

    std::string wkt {factory.create_point(osmium::Location(3.2, 4.2))};
    BOOST_CHECK_EQUAL(std::string{"POINT(3.2 4.2)"}, wkt);
}

BOOST_AUTO_TEST_CASE(empty_point) {
    osmium::geom::WKTFactory factory;

    BOOST_CHECK_THROW(factory.create_point(osmium::Location()), osmium::geom::geometry_error);
}

BOOST_AUTO_TEST_CASE(linestring) {
    osmium::geom::WKTFactory factory;

    osmium::memory::Buffer buffer(10000);
    osmium::Way& way = buffer_add_way(buffer,
        "foo",
        {},
        {
            {1, {3.2, 4.2}},
            {3, {3.5, 4.7}},
            {4, {3.5, 4.7}},
            {2, {3.6, 4.9}}
        });

    {
        std::string wkt {factory.create_linestring(way.nodes())};
        BOOST_CHECK_EQUAL(std::string{"LINESTRING(3.2 4.2,3.5 4.7,3.6 4.9)"}, wkt);
    }

    {
        std::string wkt {factory.create_linestring(way.nodes(), true, true)};
        BOOST_CHECK_EQUAL(std::string{"LINESTRING(3.6 4.9,3.5 4.7,3.2 4.2)"}, wkt);
    }

    {
        std::string wkt {factory.create_linestring(way.nodes(), false)};
        BOOST_CHECK_EQUAL(std::string{"LINESTRING(3.2 4.2,3.5 4.7,3.5 4.7,3.6 4.9)"}, wkt);
    }

    {
        std::string wkt {factory.create_linestring(way.nodes(), false, true)};
        BOOST_CHECK_EQUAL(std::string{"LINESTRING(3.6 4.9,3.5 4.7,3.5 4.7,3.2 4.2)"}, wkt);
    }
}

BOOST_AUTO_TEST_CASE(empty_linestring) {
    osmium::geom::WKTFactory factory;

    osmium::memory::Buffer buffer(10000);
    osmium::Way& way = buffer_add_way(buffer,
        "foo",
        {},
        std::vector<std::pair<osmium::object_id_type, osmium::Location>>({}));

    BOOST_CHECK_THROW(factory.create_linestring(way.nodes()), osmium::geom::geometry_error);
    BOOST_CHECK_THROW(factory.create_linestring(way.nodes(), true, true), osmium::geom::geometry_error);
    BOOST_CHECK_THROW(factory.create_linestring(way.nodes(), false), osmium::geom::geometry_error);
    BOOST_CHECK_THROW(factory.create_linestring(way.nodes(), false, true), osmium::geom::geometry_error);
}

BOOST_AUTO_TEST_CASE(linestring_with_two_same_locations) {
    osmium::geom::WKTFactory factory;

    osmium::memory::Buffer buffer(10000);
    osmium::Way& way = buffer_add_way(buffer,
        "foo",
        {},
        {
            {1, {3.5, 4.7}},
            {2, {3.5, 4.7}},
        });

    BOOST_CHECK_THROW(factory.create_linestring(way.nodes()), osmium::geom::geometry_error);
    BOOST_CHECK_THROW(factory.create_linestring(way.nodes(), true, true), osmium::geom::geometry_error);

    {
        std::string wkt {factory.create_linestring(way.nodes(), false)};
        BOOST_CHECK_EQUAL(std::string{"LINESTRING(3.5 4.7,3.5 4.7)"}, wkt);
    }

    {
        std::string wkt {factory.create_linestring(way.nodes(), false, true)};
        BOOST_CHECK_EQUAL(std::string{"LINESTRING(3.5 4.7,3.5 4.7)"}, wkt);
    }
}

BOOST_AUTO_TEST_CASE(linestring_with_undefined_location) {
    osmium::geom::WKTFactory factory;

    osmium::memory::Buffer buffer(10000);
    osmium::Way& way = buffer_add_way(buffer,
        "foo",
        {},
        {
            {1, {3.5, 4.7}},
            {2, osmium::Location()},
        });

    BOOST_CHECK_THROW(factory.create_linestring(way.nodes()), osmium::geom::geometry_error);
}

BOOST_AUTO_TEST_SUITE_END()

