#include "catch.hpp"

#include <osmium/memory/item.hpp>

TEST_CASE("padded length") {
    REQUIRE(osmium::memory::padded_length(0) ==  0);
    REQUIRE(osmium::memory::padded_length(1) ==  8);
    REQUIRE(osmium::memory::padded_length(2) ==  8);
    REQUIRE(osmium::memory::padded_length(7) ==  8);
    REQUIRE(osmium::memory::padded_length(8) ==  8);
    REQUIRE(osmium::memory::padded_length(9) == 16);

    REQUIRE(osmium::memory::padded_length(2147483647) == 2147483648);
    REQUIRE(osmium::memory::padded_length(2147483648) == 2147483648);
    REQUIRE(osmium::memory::padded_length(2147483650) == 2147483656);

    REQUIRE(osmium::memory::padded_length(4294967295) == 4294967296);
    REQUIRE(osmium::memory::padded_length(4294967296) == 4294967296);
    REQUIRE(osmium::memory::padded_length(4294967297) == 4294967304);

    REQUIRE(osmium::memory::padded_length(7999999999) == 8000000000);
    REQUIRE(osmium::memory::padded_length(8000000000) == 8000000000);
    REQUIRE(osmium::memory::padded_length(8000000001) == 8000000008);
}

