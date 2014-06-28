#ifdef STAND_ALONE
# define BOOST_TEST_MODULE Main
#endif
#include <boost/test/unit_test.hpp>

#include <osmium/geom/geos.hpp>
#include <osmium/geom/mercator_projection.hpp>
#include <osmium/geom/projection.hpp>
#include <osmium/geom/wkb.hpp>
#include <osmium/geom/wkt.hpp>

#include "helper.hpp"

BOOST_AUTO_TEST_SUITE(Projection)

BOOST_AUTO_TEST_CASE(point_mercator) {
    osmium::geom::WKTFactory<osmium::geom::MercatorProjection> factory;

    std::string wkt {factory.create_point(osmium::Location(3.2, 4.2))};
    BOOST_CHECK_EQUAL(std::string{"POINT(356222.370538 467961.143605)"}, wkt);
}

BOOST_AUTO_TEST_CASE(point_epsg_3857) {
    osmium::geom::WKTFactory<osmium::geom::Projection> factory(osmium::geom::Projection(3857));

    std::string wkt {factory.create_point(osmium::Location(3.2, 4.2))};
    BOOST_CHECK_EQUAL(std::string{"POINT(356222.370538 467961.143605)"}, wkt);
}

BOOST_AUTO_TEST_CASE(wkb_with_parameter) {
    osmium::geom::WKBFactory<osmium::geom::Projection> wkb_factory(osmium::geom::Projection(3857), osmium::geom::wkb_type::wkb, osmium::geom::out_type::hex);
    osmium::geom::GEOSFactory<osmium::geom::Projection> geos_factory(osmium::geom::Projection(3857));

    std::string wkb = wkb_factory.create_point(osmium::Location(3.2, 4.2));
    std::unique_ptr<geos::geom::Point> geos_point = geos_factory.create_point(osmium::Location(3.2, 4.2));
    BOOST_CHECK_EQUAL(geos_to_wkb(geos_point.get()), wkb);
}

BOOST_AUTO_TEST_CASE(cleanup) {
    // trying to make valgrind happy, but there is still a memory leak in proj library
    pj_deallocate_grids();
}

BOOST_AUTO_TEST_SUITE_END()
