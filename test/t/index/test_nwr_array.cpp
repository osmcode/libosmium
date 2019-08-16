#include "catch.hpp"

#include <osmium/index/nwr_array.hpp>

TEST_CASE("nwr_array") {
    osmium::nwr_array<int> a;
    a(osmium::item_type::node) = 1;
    a(osmium::item_type::way) = 2;
    a(osmium::item_type::relation) = 3;

    REQUIRE(a(osmium::item_type::node) == 1);
    REQUIRE(a(osmium::item_type::way) == 2);
    REQUIRE(a(osmium::item_type::relation) == 3);
}

