#include "catch.hpp"

#include <osmium/geom/mercator_projection.hpp>
#include <osmium/geom/projection.hpp>
#include <osmium/geom/wkb.hpp>
#include <osmium/geom/wkt.hpp>

TEST_CASE("Projection using MercatorProjection class to WKT") {
    osmium::geom::WKTFactory<osmium::geom::MercatorProjection> factory{2};

    const std::string wkt{factory.create_point(osmium::Location{3.2, 4.2})};
    REQUIRE(wkt == "POINT(356222.37 467961.14)");
}

TEST_CASE("Projection using Projection class to WKT") {
    osmium::geom::WKTFactory<osmium::geom::Projection> factory{osmium::geom::Projection{3857}, 2};

    const std::string wkt{factory.create_point(osmium::Location{3.2, 4.2})};
    REQUIRE(wkt == "POINT(356222.37 467961.14)");
}

#if __BYTE_ORDER == __LITTLE_ENDIAN

TEST_CASE("Projection using Projection class to WKB") {
    osmium::geom::WKBFactory<osmium::geom::Projection> factory{osmium::geom::Projection{3857}, osmium::geom::wkb_type::wkb, osmium::geom::out_type::hex};

    const std::string wkb{factory.create_point(osmium::Location{3.2, 4.2})};
    REQUIRE(wkb == "010100000030706E7BF9BD1541B03E0D93E48F1C41");
}

#endif

