#ifdef STAND_ALONE
# define BOOST_TEST_MODULE Main
#endif
#include <boost/test/unit_test.hpp>

#include <osmium/geom/mercator_projection.hpp>
#include <osmium/geom/projection.hpp>

BOOST_AUTO_TEST_SUITE(Projection)

BOOST_AUTO_TEST_CASE(project_location_4326) {
    osmium::geom::Projection projection(4326);

    const osmium::Location loc(1.0, 2.0);
    const osmium::geom::Coordinates c {osmium::geom::deg_to_rad(1.0), osmium::geom::deg_to_rad(2.0)};
    BOOST_CHECK_EQUAL(c, projection(loc));
}

BOOST_AUTO_TEST_CASE(project_location_4326_string) {
    osmium::geom::Projection projection("+init=epsg:4326");

    const osmium::Location loc(1.0, 2.0);
    const osmium::geom::Coordinates c {osmium::geom::deg_to_rad(1.0), osmium::geom::deg_to_rad(2.0)};
    BOOST_CHECK_EQUAL(c, projection(loc));
}

BOOST_AUTO_TEST_CASE(unknown_projection_string) {
    BOOST_CHECK_THROW(osmium::geom::Projection projection("abc"), osmium::projection_error);
}

BOOST_AUTO_TEST_CASE(unknown_epsg_code) {
    BOOST_CHECK_THROW(osmium::geom::Projection projection(9999999), osmium::projection_error);
}

BOOST_AUTO_TEST_CASE(project_location_3857) {
    osmium::geom::Projection projection(3857);

    {
        const osmium::Location loc(0.0, 0.0);
        const osmium::geom::Coordinates c {0.0, 0.0};
        BOOST_CHECK_LT(std::abs(projection(loc).x - c.x), 0.1);
        BOOST_CHECK_LT(std::abs(projection(loc).y - c.y), 0.1);
    }
    {
        const osmium::Location loc(180.0, 0.0);
        const osmium::geom::Coordinates c {20037508.34, 0.0};
        BOOST_CHECK_LT(std::abs(projection(loc).x - c.x), 0.1);
        BOOST_CHECK_LT(std::abs(projection(loc).y - c.y), 0.1);
    }
    {
        const osmium::Location loc(180.0, 0.0);
        const osmium::geom::Coordinates c {20037508.34, 0.0};
        BOOST_CHECK_LT(std::abs(projection(loc).x - c.x), 0.1);
        BOOST_CHECK_LT(std::abs(projection(loc).y - c.y), 0.1);
    }
    {
        const osmium::Location loc(0.0, 85.0511288);
        const osmium::geom::Coordinates c {0.0, 20037508.34};
        BOOST_CHECK_LT(std::abs(projection(loc).x - c.x), 0.1);
        BOOST_CHECK_LT(std::abs(projection(loc).y - c.y), 0.1);
    }
}

BOOST_AUTO_TEST_CASE(project_location_mercator) {
    osmium::geom::MercatorProjection projection;

    {
        const osmium::Location loc(0.0, 0.0);
        const osmium::geom::Coordinates c {0.0, 0.0};
        BOOST_CHECK_LT(std::abs(projection(loc).x - c.x), 0.1);
        BOOST_CHECK_LT(std::abs(projection(loc).y - c.y), 0.1);
    }
    {
        const osmium::Location loc(180.0, 0.0);
        const osmium::geom::Coordinates c {20037508.34, 0.0};
        BOOST_CHECK_LT(std::abs(projection(loc).x - c.x), 0.1);
        BOOST_CHECK_LT(std::abs(projection(loc).y - c.y), 0.1);
    }
    {
        const osmium::Location loc(180.0, 0.0);
        const osmium::geom::Coordinates c {20037508.34, 0.0};
        BOOST_CHECK_LT(std::abs(projection(loc).x - c.x), 0.1);
        BOOST_CHECK_LT(std::abs(projection(loc).y - c.y), 0.1);
    }
    {
        const osmium::Location loc(0.0, 85.0511288);
        const osmium::geom::Coordinates c {0.0, 20037508.34};
        BOOST_CHECK_LT(std::abs(projection(loc).x - c.x), 0.1);
        BOOST_CHECK_LT(std::abs(projection(loc).y - c.y), 0.1);
    }
}

BOOST_AUTO_TEST_CASE(compare_mercators) {
    osmium::geom::MercatorProjection projection_merc;
    osmium::geom::Projection projection_3857(3857);
    {
        const osmium::Location loc(4.2, 27.3);
        BOOST_CHECK_LT(std::abs(projection_merc(loc).x - projection_3857(loc).x), 0.1);
        BOOST_CHECK_LT(std::abs(projection_merc(loc).y - projection_3857(loc).y), 0.1);
    }
    {
        const osmium::Location loc(160.789, -42.42);
        BOOST_CHECK_LT(std::abs(projection_merc(loc).x - projection_3857(loc).x), 0.1);
        BOOST_CHECK_LT(std::abs(projection_merc(loc).y - projection_3857(loc).y), 0.1);
    }
    {
        const osmium::Location loc(-0.001, 0.001);
        BOOST_CHECK_LT(std::abs(projection_merc(loc).x - projection_3857(loc).x), 0.1);
        BOOST_CHECK_LT(std::abs(projection_merc(loc).y - projection_3857(loc).y), 0.1);
    }
    {
        const osmium::Location loc(-85.2, -85.2);
        BOOST_CHECK_LT(std::abs(projection_merc(loc).x - projection_3857(loc).x), 0.1);
        BOOST_CHECK_LT(std::abs(projection_merc(loc).y - projection_3857(loc).y), 0.1);
    }
}

BOOST_AUTO_TEST_CASE(low_level_mercator_functions) {
    BOOST_CHECK_LT(17.839 - osmium::geom::detail::x_to_lon(osmium::geom::detail::lon_to_x(17.839)), 0.000000001);
    BOOST_CHECK_LT(-3.005 - osmium::geom::detail::x_to_lon(osmium::geom::detail::lon_to_x(-3.005)), 0.000000001);
    BOOST_CHECK_LT( 180.0 - osmium::geom::detail::x_to_lon(osmium::geom::detail::lon_to_x( 180.0)), 0.000000001);

    BOOST_CHECK_LT(17.839 - osmium::geom::detail::y_to_lat(osmium::geom::detail::lat_to_y(17.839)), 0.000000001);
    BOOST_CHECK_LT(-3.005 - osmium::geom::detail::y_to_lat(osmium::geom::detail::lat_to_y(-3.005)), 0.000000001);
    BOOST_CHECK_LT(  85.0 - osmium::geom::detail::y_to_lat(osmium::geom::detail::lat_to_y(  85.0)), 0.000000001);
}

BOOST_AUTO_TEST_SUITE_END()
