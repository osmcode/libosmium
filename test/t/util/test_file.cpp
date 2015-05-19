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

