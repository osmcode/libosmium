#ifdef STAND_ALONE
# define BOOST_TEST_MODULE Main
#endif
#include <boost/test/unit_test.hpp>

#include <osmium/builder/builder_helper.hpp>
#include <osmium/geom/wkb.hpp>

#include "../basic/helper.hpp"

BOOST_AUTO_TEST_SUITE(WKB_Geometry)

BOOST_AUTO_TEST_CASE(point) {
    osmium::geom::WKBFactory<> factory(osmium::geom::wkb_type::wkb, osmium::geom::out_type::hex);

    std::string wkb {factory.create_point(osmium::Location(3.2, 4.2))};
    BOOST_CHECK_EQUAL(std::string{"01010000009A99999999990940CDCCCCCCCCCC1040"}, wkb);
}

BOOST_AUTO_TEST_CASE(point_ewkb) {
    osmium::geom::WKBFactory<> factory(osmium::geom::wkb_type::ewkb, osmium::geom::out_type::hex);

    std::string wkb {factory.create_point(osmium::Location(3.2, 4.2))};
    BOOST_CHECK_EQUAL(std::string{"0101000020E61000009A99999999990940CDCCCCCCCCCC1040"}, wkb);
}

BOOST_AUTO_TEST_CASE(empty_point) {
    osmium::geom::WKBFactory<> factory(osmium::geom::wkb_type::wkb, osmium::geom::out_type::hex);

    BOOST_CHECK_THROW(factory.create_point(osmium::Location()), osmium::invalid_location);
}

BOOST_AUTO_TEST_CASE(linestring) {
    osmium::geom::WKBFactory<> factory(osmium::geom::wkb_type::wkb, osmium::geom::out_type::hex);

    osmium::memory::Buffer buffer(10000);
    auto& wnl = osmium::builder::build_way_node_list(buffer, {
        {1, {3.2, 4.2}},
        {3, {3.5, 4.7}},
        {4, {3.5, 4.7}},
        {2, {3.6, 4.9}}
    });

    {
        std::string wkb {factory.create_linestring(wnl)};
        BOOST_CHECK_EQUAL(std::string{"0102000000030000009A99999999990940CDCCCCCCCCCC10400000000000000C40CDCCCCCCCCCC1240CDCCCCCCCCCC0C409A99999999991340"}, wkb);
    }

    {
        std::string wkb {factory.create_linestring(wnl, osmium::geom::use_nodes::unique, osmium::geom::direction::backward)};
        BOOST_CHECK_EQUAL(std::string{"010200000003000000CDCCCCCCCCCC0C409A999999999913400000000000000C40CDCCCCCCCCCC12409A99999999990940CDCCCCCCCCCC1040"}, wkb);
    }

    {
        std::string wkb {factory.create_linestring(wnl, osmium::geom::use_nodes::all)};
        BOOST_CHECK_EQUAL(std::string{"0102000000040000009A99999999990940CDCCCCCCCCCC10400000000000000C40CDCCCCCCCCCC12400000000000000C40CDCCCCCCCCCC1240CDCCCCCCCCCC0C409A99999999991340"}, wkb);
    }

    {
        std::string wkb {factory.create_linestring(wnl, osmium::geom::use_nodes::all, osmium::geom::direction::backward)};
        BOOST_CHECK_EQUAL(std::string{"010200000004000000CDCCCCCCCCCC0C409A999999999913400000000000000C40CDCCCCCCCCCC12400000000000000C40CDCCCCCCCCCC12409A99999999990940CDCCCCCCCCCC1040"}, wkb);
    }
}

BOOST_AUTO_TEST_CASE(linestring_ewkb) {
    osmium::geom::WKBFactory<> factory(osmium::geom::wkb_type::ewkb, osmium::geom::out_type::hex);

    osmium::memory::Buffer buffer(10000);
    auto& wnl = osmium::builder::build_way_node_list(buffer, {
        {1, {3.2, 4.2}},
        {3, {3.5, 4.7}},
        {4, {3.5, 4.7}},
        {2, {3.6, 4.9}}
    });

    std::string ewkb {factory.create_linestring(wnl)};
    BOOST_CHECK_EQUAL(std::string{"0102000020E6100000030000009A99999999990940CDCCCCCCCCCC10400000000000000C40CDCCCCCCCCCC1240CDCCCCCCCCCC0C409A99999999991340"}, ewkb);
}

BOOST_AUTO_TEST_CASE(empty_linestring) {
    osmium::geom::WKBFactory<> factory(osmium::geom::wkb_type::wkb, osmium::geom::out_type::hex);

    osmium::memory::Buffer buffer(10000);
    auto& wnl = osmium::builder::build_way_node_list(buffer, {});

    BOOST_CHECK_THROW(factory.create_linestring(wnl), osmium::geometry_error);
    BOOST_CHECK_THROW(factory.create_linestring(wnl, osmium::geom::use_nodes::unique, osmium::geom::direction::backward), osmium::geometry_error);
    BOOST_CHECK_THROW(factory.create_linestring(wnl, osmium::geom::use_nodes::all), osmium::geometry_error);
    BOOST_CHECK_THROW(factory.create_linestring(wnl, osmium::geom::use_nodes::all, osmium::geom::direction::backward), osmium::geometry_error);
}

BOOST_AUTO_TEST_CASE(linestring_with_two_same_locations) {
    osmium::geom::WKBFactory<> factory(osmium::geom::wkb_type::wkb, osmium::geom::out_type::hex);

    osmium::memory::Buffer buffer(10000);
    auto& wnl = osmium::builder::build_way_node_list(buffer, {
        {1, {3.5, 4.7}},
        {2, {3.5, 4.7}}
    });

    BOOST_CHECK_THROW(factory.create_linestring(wnl), osmium::geometry_error);
    BOOST_CHECK_THROW(factory.create_linestring(wnl, osmium::geom::use_nodes::unique, osmium::geom::direction::backward), osmium::geometry_error);

    {
        std::string wkb {factory.create_linestring(wnl, osmium::geom::use_nodes::all)};
        BOOST_CHECK_EQUAL(std::string{"0102000000020000000000000000000C40CDCCCCCCCCCC12400000000000000C40CDCCCCCCCCCC1240"}, wkb);
    }

    {
        std::string wkb {factory.create_linestring(wnl, osmium::geom::use_nodes::all, osmium::geom::direction::backward)};
        BOOST_CHECK_EQUAL(std::string{"0102000000020000000000000000000C40CDCCCCCCCCCC12400000000000000C40CDCCCCCCCCCC1240"}, wkb);
    }
}

BOOST_AUTO_TEST_CASE(linestring_with_undefined_location) {
    osmium::geom::WKBFactory<> factory(osmium::geom::wkb_type::wkb, osmium::geom::out_type::hex);

    osmium::memory::Buffer buffer(10000);
    auto& wnl = osmium::builder::build_way_node_list(buffer, {
        {1, {3.5, 4.7}},
        {2, osmium::Location()}
    });

    BOOST_CHECK_THROW(factory.create_linestring(wnl), osmium::invalid_location);
}

BOOST_AUTO_TEST_SUITE_END()

