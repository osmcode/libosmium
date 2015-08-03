#include "catch.hpp"

#include <boost/crc.hpp>

#include <osmium/osm/crc.hpp>

#include "helper.hpp"

TEST_CASE("CRC of basic datatypes") {

    osmium::CRC<boost::crc_32_type> crc32;

    SECTION("Bool") {
        crc32.update(true);
        crc32.update(false);

        REQUIRE(crc32().checksum() == 0x58c223be);
    }

    SECTION("Char") {
        crc32.update('x');
        crc32.update('y');

        REQUIRE(crc32().checksum() == 0xca0ba5a);
    }

    SECTION("String") {
        const char* str = "foobar";
        crc32.update(str);

        REQUIRE(crc32().checksum() == 0x9ef61f95);
    }

    SECTION("Timestamp") {
        osmium::Timestamp t("2015-07-12T13:10:46Z");
        crc32.update(t);

        REQUIRE(crc32().checksum() == 0x1c5da71d);
    }

    SECTION("Location") {
        osmium::Location loc { 3.46, 2.001 };
        crc32.update(loc);

        REQUIRE(crc32().checksum() == 0xddee042c);
    }

}

