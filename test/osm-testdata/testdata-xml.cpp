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

struct header_buffer_type {
    osmium::io::Header header;
    osmium::memory::Buffer buffer;
};

// This helper function is used to parse XML data without the usual threading.
// It is only for use in testing because it makes some assumptions which will
// not always be true in normal code.
header_buffer_type read_xml(const char* test_id) {
    osmium::thread::Queue<std::string> input_queue;
    osmium::thread::Queue<osmium::memory::Buffer> output_queue;
    std::promise<osmium::io::Header> header_promise;
    std::atomic<bool> done {false};

    osmium::io::detail::XMLParser parser(input_queue, output_queue, header_promise, osmium::osm_entity_bits::all, done);

    int fd = osmium::io::detail::open_for_reading(filename(test_id));
    assert(fd >= 0);
    std::string input(10000, '\0');
    int n = ::read(fd, reinterpret_cast<unsigned char*>(const_cast<char*>(input.data())), 10000);
    assert(n >= 0);
    input.resize(n);
    input_queue.push(input);
    input_queue.push(std::string());

//    std::cerr << "call parser for " << test_id << ":\n";
    parser();

    header_buffer_type result;
//    std::cerr << "  get header...\n";
    result.header = header_promise.get_future().get();
//    std::cerr << "  get buffer...\n";
    output_queue.wait_and_pop(result.buffer);

//    std::cerr << "  check is done...\n";
    if (result.buffer) {
        osmium::memory::Buffer buffer;
        output_queue.wait_and_pop(buffer);
        assert(!buffer);
    }

//    std::cerr << "  DONE\n";
    close(fd);

    return result;
}


TEST_CASE("Reading OSM XML") {

SECTION("Test_100") {
    header_buffer_type r = read_xml("100");

    REQUIRE(r.header.get("generator") == "testdata");
    REQUIRE(0 == r.buffer.committed());
    REQUIRE(! r.buffer);
}

SECTION("Test_100 with Reader") {
    osmium::io::Reader reader(filename("100"));

    osmium::io::Header header = reader.header();
    REQUIRE(header.get("generator") == "testdata");

    osmium::memory::Buffer buffer = reader.read();
    REQUIRE(0 == buffer.committed());
    REQUIRE(! buffer);
}

SECTION("Test_101") {
    REQUIRE_THROWS_AS(read_xml("101"), std::runtime_error);
}

SECTION("Test_102") {
    REQUIRE_THROWS_AS(read_xml("102"), std::runtime_error);
}

SECTION("Test_103") {
    REQUIRE_THROWS_AS(read_xml("103"), std::runtime_error);
}

SECTION("Test_104") {
    REQUIRE_THROWS_AS(read_xml("104"), std::runtime_error);
}

SECTION("Test_105") {
    REQUIRE_THROWS_AS(read_xml("105"), std::runtime_error);
}

}

