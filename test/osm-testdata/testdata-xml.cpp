/* The code in this file is released into the Public Domain. */

#define BOOST_TEST_MODULE Main

#include <boost/test/unit_test.hpp>
#include <boost/test/output_test_stream.hpp>
using boost::test_tools::output_test_stream;

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


BOOST_AUTO_TEST_SUITE(OSM_XML_Format)

BOOST_AUTO_TEST_CASE(Test_100) {
    osmium::io::Reader reader(filename("100"));

    osmium::io::Header header = reader.header();
    BOOST_CHECK_EQUAL(header.get("generator"), "testdata");

    osmium::memory::Buffer buffer = reader.read();
    BOOST_CHECK_EQUAL(0, buffer.committed());
}

#if 0
BOOST_AUTO_TEST_CASE(Test_101) {
    osmium::io::Reader reader(filename("101"));
    BOOST_CHECK_THROW(osmium::io::Header header = reader.header(), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(Test_102) {
    osmium::io::Reader reader(filename("102"));
    BOOST_CHECK_THROW(osmium::io::Header header = reader.header(), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(Test_103) {
    osmium::io::Reader reader(filename("103"));
    BOOST_CHECK_THROW(osmium::io::Header header = reader.header(), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(Test_104) {
    osmium::io::Reader reader(filename("104"));
    BOOST_CHECK_THROW(osmium::io::Header header = reader.header(), std::runtime_error);
}
#endif

BOOST_AUTO_TEST_SUITE_END()

