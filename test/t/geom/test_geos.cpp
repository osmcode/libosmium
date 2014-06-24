#ifdef STAND_ALONE
# define BOOST_TEST_MODULE Main
#endif
#include <boost/test/unit_test.hpp>

#include <osmium/builder/builder_helper.hpp>
#include <osmium/geom/geos.hpp>

#include "../basic/helper.hpp"

BOOST_AUTO_TEST_SUITE(GEOS_Geometry)

BOOST_AUTO_TEST_CASE(point) {
    osmium::geom::GEOSFactory factory;

    std::unique_ptr<geos::geom::Point> point {factory.create_point(osmium::Location(3.2, 4.2))};
    BOOST_CHECK_EQUAL(3.2, point->getX());
    BOOST_CHECK_EQUAL(4.2, point->getY());
}

BOOST_AUTO_TEST_CASE(empty_point) {
    osmium::geom::GEOSFactory factory;

    BOOST_CHECK_THROW(factory.create_point(osmium::Location()), osmium::invalid_location);
}

BOOST_AUTO_TEST_CASE(linestring) {
    osmium::geom::GEOSFactory factory;

    osmium::memory::Buffer buffer(10000);
    auto& wnl = osmium::builder::build_way_node_list(buffer, {
        {1, {3.2, 4.2}},
        {3, {3.5, 4.7}},
        {4, {3.5, 4.7}},
        {2, {3.6, 4.9}}
    });

    {
        std::unique_ptr<geos::geom::LineString> linestring {factory.create_linestring(wnl)};
        BOOST_CHECK_EQUAL(3, linestring->getNumPoints());

        std::unique_ptr<geos::geom::Point> p0 = std::unique_ptr<geos::geom::Point>(linestring->getPointN(0));
        BOOST_CHECK_EQUAL(3.2, p0->getX());
        std::unique_ptr<geos::geom::Point> p2 = std::unique_ptr<geos::geom::Point>(linestring->getPointN(2));
        BOOST_CHECK_EQUAL(3.6, p2->getX());
    }

    {
        std::unique_ptr<geos::geom::LineString> linestring {factory.create_linestring(wnl, osmium::geom::use_nodes::unique, osmium::geom::direction::backward)};
        BOOST_CHECK_EQUAL(3, linestring->getNumPoints());
        std::unique_ptr<geos::geom::Point> p0 = std::unique_ptr<geos::geom::Point>(linestring->getPointN(0));
        BOOST_CHECK_EQUAL(3.6, p0->getX());
        std::unique_ptr<geos::geom::Point> p2 = std::unique_ptr<geos::geom::Point>(linestring->getPointN(2));
        BOOST_CHECK_EQUAL(3.2, p2->getX());
    }

    {
        std::unique_ptr<geos::geom::LineString> linestring {factory.create_linestring(wnl, osmium::geom::use_nodes::all)};
        BOOST_CHECK_EQUAL(4, linestring->getNumPoints());
        std::unique_ptr<geos::geom::Point> p0 = std::unique_ptr<geos::geom::Point>(linestring->getPointN(0));
        BOOST_CHECK_EQUAL(3.2, p0->getX());
    }

    {
        std::unique_ptr<geos::geom::LineString> linestring {factory.create_linestring(wnl, osmium::geom::use_nodes::all, osmium::geom::direction::backward)};
        BOOST_CHECK_EQUAL(4, linestring->getNumPoints());
        std::unique_ptr<geos::geom::Point> p0 = std::unique_ptr<geos::geom::Point>(linestring->getPointN(0));
        BOOST_CHECK_EQUAL(3.6, p0->getX());
    }
}

BOOST_AUTO_TEST_CASE(area_1outer_0inner) {
    osmium::geom::GEOSFactory factory;

    osmium::memory::Buffer buffer(10000);
    osmium::Area& area = buffer_add_area(buffer,
        "foo",
        {},
        {
            { true, {
                {1, {3.2, 4.2}},
                {2, {3.5, 4.7}},
                {3, {3.6, 4.9}},
                {1, {3.2, 4.2}}
            }}
        });

    std::unique_ptr<geos::geom::MultiPolygon> mp {factory.create_multipolygon(area)};
    BOOST_CHECK_EQUAL(1, mp->getNumGeometries());

    const geos::geom::Polygon* p0 = dynamic_cast<const geos::geom::Polygon*>(mp->getGeometryN(0));
    BOOST_CHECK_EQUAL(0, p0->getNumInteriorRing());

    const geos::geom::LineString* l0e = p0->getExteriorRing();
    BOOST_CHECK_EQUAL(4, l0e->getNumPoints());

    std::unique_ptr<geos::geom::Point> l0e_p0 = std::unique_ptr<geos::geom::Point>(l0e->getPointN(1));
    BOOST_CHECK_EQUAL(3.5, l0e_p0->getX());
}

BOOST_AUTO_TEST_CASE(area_1outer_1inner) {
    osmium::geom::GEOSFactory factory;

    osmium::memory::Buffer buffer(10000);
    osmium::Area& area = buffer_add_area(buffer,
        "foo",
        {},
        {
            { true, {
                {1, {0.1, 0.1}},
                {2, {9.1, 0.1}},
                {3, {9.1, 9.1}},
                {4, {0.1, 9.1}},
                {1, {0.1, 0.1}},
            }},
            { false, {
                {5, {1.0, 1.0}},
                {6, {8.0, 1.0}},
                {7, {8.0, 8.0}},
                {8, {1.0, 8.0}},
                {5, {1.0, 1.0}},
            }}
        });

    std::unique_ptr<geos::geom::MultiPolygon> mp {factory.create_multipolygon(area)};
    BOOST_CHECK_EQUAL(1, mp->getNumGeometries());

    const geos::geom::Polygon* p0 = dynamic_cast<const geos::geom::Polygon*>(mp->getGeometryN(0));
    BOOST_CHECK_EQUAL(1, p0->getNumInteriorRing());

    const geos::geom::LineString* l0e = p0->getExteriorRing();
    BOOST_CHECK_EQUAL(5, l0e->getNumPoints());

    const geos::geom::LineString* l0i0 = p0->getInteriorRingN(0);
    BOOST_CHECK_EQUAL(5, l0i0->getNumPoints());
}

BOOST_AUTO_TEST_CASE(area_2outer_2inner) {
    osmium::geom::GEOSFactory factory;

    osmium::memory::Buffer buffer(10000);
    osmium::Area& area = buffer_add_area(buffer,
        "foo",
        {},
        {
            { true, {
                {1, {0.1, 0.1}},
                {2, {9.1, 0.1}},
                {3, {9.1, 9.1}},
                {4, {0.1, 9.1}},
                {1, {0.1, 0.1}}
            }},
            { false, {
                {5, {1.0, 1.0}},
                {6, {4.0, 1.0}},
                {7, {4.0, 4.0}},
                {8, {1.0, 4.0}},
                {5, {1.0, 1.0}}
            }},
            { false, {
                {10, {5.0, 5.0}},
                {11, {5.0, 7.0}},
                {12, {7.0, 7.0}},
                {10, {5.0, 5.0}}
            }},
            { true, {
                {100, {10.0, 10.0}},
                {101, {11.0, 10.0}},
                {102, {11.0, 11.0}},
                {103, {10.0, 11.0}},
                {100, {10.0, 10.0}}
            }}
        });

    std::unique_ptr<geos::geom::MultiPolygon> mp {factory.create_multipolygon(area)};
    BOOST_CHECK_EQUAL(2, mp->getNumGeometries());

    const geos::geom::Polygon* p0 = dynamic_cast<const geos::geom::Polygon*>(mp->getGeometryN(0));
    BOOST_CHECK_EQUAL(2, p0->getNumInteriorRing());

    const geos::geom::LineString* l0e = p0->getExteriorRing();
    BOOST_CHECK_EQUAL(5, l0e->getNumPoints());

    const geos::geom::Polygon* p1 = dynamic_cast<const geos::geom::Polygon*>(mp->getGeometryN(1));
    BOOST_CHECK_EQUAL(0, p1->getNumInteriorRing());

    const geos::geom::LineString* l1e = p1->getExteriorRing();
    BOOST_CHECK_EQUAL(5, l1e->getNumPoints());
}

BOOST_AUTO_TEST_SUITE_END()
