#include "catch.hpp"

#include <osmium/util/file.hpp>

TEST_CASE("pagesize") {

    SECTION("round to pagesize") {
        size_t ps = osmium::util::get_pagesize();
        REQUIRE(osmium::util::round_to_pagesize(   0) == ps);
        REQUIRE(osmium::util::round_to_pagesize(   1) == ps);
        REQUIRE(osmium::util::round_to_pagesize(ps-1) == ps);
        REQUIRE(osmium::util::round_to_pagesize(  ps) == ps);
        REQUIRE(osmium::util::round_to_pagesize(ps+1) == ps * 2);
    }

}

TEST_CASE("file_size") {

    SECTION("illegal fd should throw") {
        REQUIRE_THROWS_AS(osmium::util::file_size(-1), std::system_error);
    }

    SECTION("unused fd should throw") {
        // its unlikely that fd 1000 is open...
        REQUIRE_THROWS_AS(osmium::util::file_size(1000), std::system_error);
    }

}

#if 0
TEST_CASE("resize_file") {

    SECTION("illegal fd should throw") {
        REQUIRE_THROWS_AS(osmium::util::resize_file(-1, 10), std::system_error);
    }

    SECTION("unused fd should throw") {
        // its unlikely that fd 1000 is open...
        REQUIRE_THROWS_AS(osmium::util::resize_file(1000, 10), std::system_error);
    }

}
#endif

