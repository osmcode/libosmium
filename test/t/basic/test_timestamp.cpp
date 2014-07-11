#include "catch.hpp"

#include <sstream>

#include <osmium/osm/timestamp.hpp>

TEST_CASE("Timestamp") {

SECTION("can_be_default_initialized") {
    osmium::Timestamp t;
    REQUIRE(0 == t);
    REQUIRE("" == t.to_iso());
}

SECTION("invalid_value") {
    osmium::Timestamp t(static_cast<time_t>(0));
    REQUIRE(0 == t);
    REQUIRE("" == t.to_iso());
}

SECTION("can_be_initialized_from_time_t") {
    osmium::Timestamp t(static_cast<time_t>(1));
    REQUIRE(1 == t);
    REQUIRE("1970-01-01T00:00:01Z" == t.to_iso());
}

SECTION("can_be_compared") {
    osmium::Timestamp t1(10);
    osmium::Timestamp t2(50);
    REQUIRE(t1 < t2);
}

SECTION("can_be_written_to_stream") {
    std::stringstream ss;
    osmium::Timestamp t(1);
    ss << t;
    REQUIRE("1970-01-01T00:00:01Z" == ss.str());
}

}
