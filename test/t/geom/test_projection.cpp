#ifdef STAND_ALONE
# define BOOST_TEST_MODULE Main
#endif
#include <boost/test/unit_test.hpp>

#include <osmium/geom/factory.hpp>
#include <osmium/geom/mercator_projection.hpp>
#include <osmium/geom/projection.hpp>

BOOST_AUTO_TEST_SUITE(Projection)

BOOST_AUTO_TEST_CASE(identity_projection) {
    osmium::geom::IdentityProjection projection;
    BOOST_CHECK_EQUAL(4326, projection.epsg());
    BOOST_CHECK_EQUAL("+proj=longlat +datum=WGS84 +no_defs", projection.proj_string());
}

BOOST_AUTO_TEST_CASE(project_location_4326) {
    osmium::geom::Projection projection(4326);
    BOOST_CHECK_EQUAL(4326, projection.epsg());
    BOOST_CHECK_EQUAL("+init=epsg:4326", projection.proj_string());

    const osmium::Location loc(1.0, 2.0);
    const osmium::geom::Coordinates c {1.0, 2.0};
    BOOST_CHECK_EQUAL(c, projection(loc));
}

BOOST_AUTO_TEST_CASE(project_location_4326_string) {
    osmium::geom::Projection projection("+init=epsg:4326");
    BOOST_CHECK_EQUAL(-1, projection.epsg());
    BOOST_CHECK_EQUAL("+init=epsg:4326", projection.proj_string());

    const osmium::Location loc(1.0, 2.0);
    const osmium::geom::Coordinates c {1.0, 2.0};
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
    BOOST_CHECK_EQUAL(3857, projection.epsg());
    BOOST_CHECK_EQUAL("+init=epsg:3857", projection.proj_string());

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
    BOOST_CHECK_EQUAL(3857, projection_3857.epsg());
    BOOST_CHECK_EQUAL("+init=epsg:3857", projection_3857.proj_string());

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

BOOST_AUTO_TEST_SUITE_END()
