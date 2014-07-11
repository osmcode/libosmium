/* The code in this file is released into the Public Domain. */

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <cassert>
#include <cstdlib>
#include <iostream>
#include <string>

#include <osmium/io/xml_input.hpp>
#include <osmium/visitor.hpp>

std::string filename(const char* test_id) {
    const char* testdir = getenv("TESTDIR");
    if (!testdir) {
        std::cerr << "You have to set TESTDIR environment variable before running testdata-xml\n";
        exit(2);
    }

    std::string f;
    f += testdir;
    f += "/";
    f += test_id;
    f += "/data.osm";
    return f;
}


TEST_CASE("OSM_XML_Format") {

SECTION("Test_100") {
    osmium::io::Reader reader(filename("100"));

    osmium::io::Header header = reader.header();
    REQUIRE(header.get("generator") == "testdata");

    osmium::memory::Buffer buffer = reader.read();
    REQUIRE(0 == buffer.committed());
}

#if 0
SECTION("Test_101") {
    osmium::io::Reader reader(filename("101"));
    REQUIRE_THROWS_AS(osmium::io::Header header = reader.header(), std::runtime_error);
}

SECTION("Test_102") {
    osmium::io::Reader reader(filename("102"));
    REQUIRE_THROWS_AS(osmium::io::Header header = reader.header(), std::runtime_error);
}

SECTION("Test_103") {
    osmium::io::Reader reader(filename("103"));
    REQUIRE_THROWS_AS(osmium::io::Header header = reader.header(), std::runtime_error);
}

SECTION("Test_104") {
    osmium::io::Reader reader(filename("104"));
    REQUIRE_THROWS_AS(osmium::io::Header header = reader.header(), std::runtime_error);
}
#endif

}

