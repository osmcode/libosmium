#ifdef STAND_ALONE
# define BOOST_TEST_MODULE Main
#endif
#include <boost/test/unit_test.hpp>

#include <osmium/geom/wkb.hpp>

#include "../basic/helper.hpp"

BOOST_AUTO_TEST_SUITE(WKB_Geometry)

BOOST_AUTO_TEST_CASE(point) {
    osmium::geom::WKBFactory factory;
    factory.set_hex_mode();

    std::string wkb {factory.create_point(osmium::Location(3.2, 4.2))};
    BOOST_CHECK_EQUAL(std::string{"01010000009a99999999990940cdcccccccccc1040"}, wkb);
}

BOOST_AUTO_TEST_CASE(point_ewkb) {
    osmium::geom::WKBFactory factory(true);
    factory.set_hex_mode();

    std::string wkb {factory.create_point(osmium::Location(3.2, 4.2))};
    BOOST_CHECK_EQUAL(std::string{"0101000020e61000009a99999999990940cdcccccccccc1040"}, wkb);
}

BOOST_AUTO_TEST_CASE(empty_point) {
    osmium::geom::WKBFactory factory;
    factory.set_hex_mode();

    BOOST_CHECK_THROW(factory.create_point(osmium::Location()), osmium::geom::geometry_error);
}

BOOST_AUTO_TEST_CASE(linestring) {
    osmium::geom::WKBFactory factory;
    factory.set_hex_mode();

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
        std::string wkb {factory.create_linestring(way.nodes())};
        BOOST_CHECK_EQUAL(std::string{"0102000000030000009a99999999990940cdcccccccccc10400000000000000c40cdcccccccccc1240cdcccccccccc0c409a99999999991340"}, wkb);
    }

    {
        std::string wkb {factory.create_linestring(way.nodes(), true, true)};
        BOOST_CHECK_EQUAL(std::string{"010200000003000000cdcccccccccc0c409a999999999913400000000000000c40cdcccccccccc12409a99999999990940cdcccccccccc1040"}, wkb);
    }

    {
        std::string wkb {factory.create_linestring(way.nodes(), false)};
        BOOST_CHECK_EQUAL(std::string{"0102000000040000009a99999999990940cdcccccccccc10400000000000000c40cdcccccccccc12400000000000000c40cdcccccccccc1240cdcccccccccc0c409a99999999991340"}, wkb);
    }

    {
        std::string wkb {factory.create_linestring(way.nodes(), false, true)};
        BOOST_CHECK_EQUAL(std::string{"010200000004000000cdcccccccccc0c409a999999999913400000000000000c40cdcccccccccc12400000000000000c40cdcccccccccc12409a99999999990940cdcccccccccc1040"}, wkb);
    }
}

BOOST_AUTO_TEST_CASE(linestring_ewkb) {
    osmium::geom::WKBFactory factory(true);
    factory.set_hex_mode();

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

    std::string ewkb {factory.create_linestring(way.nodes())};
    BOOST_CHECK_EQUAL(std::string{"0102000020e6100000030000009a99999999990940cdcccccccccc10400000000000000c40cdcccccccccc1240cdcccccccccc0c409a99999999991340"}, ewkb);
}

BOOST_AUTO_TEST_CASE(empty_linestring) {
    osmium::geom::WKBFactory factory;
    factory.set_hex_mode();

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
    osmium::geom::WKBFactory factory;
    factory.set_hex_mode();

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
        std::string wkb {factory.create_linestring(way.nodes(), false)};
        BOOST_CHECK_EQUAL(std::string{"0102000000020000000000000000000c40cdcccccccccc12400000000000000c40cdcccccccccc1240"}, wkb);
    }

    {
        std::string wkb {factory.create_linestring(way.nodes(), false, true)};
        BOOST_CHECK_EQUAL(std::string{"0102000000020000000000000000000c40cdcccccccccc12400000000000000c40cdcccccccccc1240"}, wkb);
    }
}

BOOST_AUTO_TEST_CASE(linestring_with_undefined_location) {
    osmium::geom::WKBFactory factory;
    factory.set_hex_mode();

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

