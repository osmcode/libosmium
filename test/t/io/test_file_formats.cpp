#ifdef STAND_ALONE
# define BOOST_TEST_MODULE Main
#endif
#include <boost/test/unit_test.hpp>
#include <boost/test/output_test_stream.hpp>
using boost::test_tools::output_test_stream;

#include <iterator>

#include <osmium/io/file.hpp>

BOOST_AUTO_TEST_SUITE(FileFormats)

BOOST_AUTO_TEST_CASE(default_file_format) {
    osmium::io::File f;
    BOOST_CHECK_EQUAL(osmium::io::file_format::unknown, f.format());
    BOOST_CHECK_EQUAL(osmium::io::file_compression::none, f.compression());
    BOOST_CHECK_EQUAL(false, f.has_multiple_object_versions());
    BOOST_CHECK_THROW(f.check(), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(stdin_stdout_empty) {
    osmium::io::File f {""};
    BOOST_CHECK_EQUAL(osmium::io::file_format::unknown, f.format());
    BOOST_CHECK_EQUAL(osmium::io::file_compression::none, f.compression());
    BOOST_CHECK_EQUAL(false, f.has_multiple_object_versions());
    BOOST_CHECK_THROW(f.check(), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(stdin_stdout_dash) {
    osmium::io::File f {"-"};
    BOOST_CHECK_EQUAL(osmium::io::file_format::unknown, f.format());
    BOOST_CHECK_EQUAL(osmium::io::file_compression::none, f.compression());
    BOOST_CHECK_EQUAL(false, f.has_multiple_object_versions());
    BOOST_CHECK_THROW(f.check(), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(stdin_stdout_bz2) {
    osmium::io::File f {"-", "osm.bz2"};
    BOOST_CHECK_EQUAL("", f.filename());
    BOOST_CHECK_EQUAL(osmium::io::file_format::xml, f.format());
    BOOST_CHECK_EQUAL(osmium::io::file_compression::bzip2, f.compression());
    BOOST_CHECK_EQUAL(false, f.has_multiple_object_versions());
    f.check();
}

BOOST_AUTO_TEST_CASE(detect_file_format_by_suffix_osm) {
    osmium::io::File f {"test.osm"};
    BOOST_CHECK_EQUAL(osmium::io::file_format::xml, f.format());
    BOOST_CHECK_EQUAL(osmium::io::file_compression::none, f.compression());
    BOOST_CHECK_EQUAL(false, f.has_multiple_object_versions());
    f.check();
}

BOOST_AUTO_TEST_CASE(detect_file_format_by_suffix_pbf) {
    osmium::io::File f {"test.pbf"};
    BOOST_CHECK_EQUAL(osmium::io::file_format::pbf, f.format());
    BOOST_CHECK_EQUAL(osmium::io::file_compression::none, f.compression());
    BOOST_CHECK_EQUAL(false, f.has_multiple_object_versions());
    f.check();
}

BOOST_AUTO_TEST_CASE(detect_file_format_by_suffix_osm_pbf) {
    osmium::io::File f {"test.osm.pbf"};
    BOOST_CHECK_EQUAL(osmium::io::file_format::pbf, f.format());
    BOOST_CHECK_EQUAL(osmium::io::file_compression::none, f.compression());
    BOOST_CHECK_EQUAL(false, f.has_multiple_object_versions());
    f.check();
}

BOOST_AUTO_TEST_CASE(detect_file_format_by_suffix_opl) {
    osmium::io::File f {"test.opl"};
    BOOST_CHECK_EQUAL(osmium::io::file_format::opl, f.format());
    BOOST_CHECK_EQUAL(osmium::io::file_compression::none, f.compression());
    BOOST_CHECK_EQUAL(false, f.has_multiple_object_versions());
    f.check();
}

BOOST_AUTO_TEST_CASE(detect_file_format_by_suffix_osm_opl) {
    osmium::io::File f {"test.osm.opl"};
    BOOST_CHECK_EQUAL(osmium::io::file_format::opl, f.format());
    BOOST_CHECK_EQUAL(osmium::io::file_compression::none, f.compression());
    BOOST_CHECK_EQUAL(false, f.has_multiple_object_versions());
    f.check();
}

BOOST_AUTO_TEST_CASE(detect_file_format_by_suffix_osm_gz) {
    osmium::io::File f {"test.osm.gz"};
    BOOST_CHECK_EQUAL(osmium::io::file_format::xml, f.format());
    BOOST_CHECK_EQUAL(osmium::io::file_compression::gzip, f.compression());
    BOOST_CHECK_EQUAL(false, f.has_multiple_object_versions());
    f.check();
}

BOOST_AUTO_TEST_CASE(detect_file_format_by_suffix_opl_bz2) {
    osmium::io::File f {"test.osm.opl.bz2"};
    BOOST_CHECK_EQUAL(osmium::io::file_format::opl, f.format());
    BOOST_CHECK_EQUAL(osmium::io::file_compression::bzip2, f.compression());
    BOOST_CHECK_EQUAL(false, f.has_multiple_object_versions());
    f.check();
}

BOOST_AUTO_TEST_CASE(detect_file_format_by_suffix_osc_gz) {
    osmium::io::File f {"test.osc.gz"};
    BOOST_CHECK_EQUAL(osmium::io::file_format::xml, f.format());
    BOOST_CHECK_EQUAL(osmium::io::file_compression::gzip, f.compression());
    BOOST_CHECK_EQUAL(true, f.has_multiple_object_versions());
    f.check();
}

BOOST_AUTO_TEST_CASE(detect_file_format_by_suffix_opl_gz) {
    osmium::io::File f {"test.osh.opl.gz"};
    BOOST_CHECK_EQUAL(osmium::io::file_format::opl, f.format());
    BOOST_CHECK_EQUAL(osmium::io::file_compression::gzip, f.compression());
    BOOST_CHECK_EQUAL(true, f.has_multiple_object_versions());
    f.check();
}

BOOST_AUTO_TEST_CASE(detect_file_format_by_suffix_osh_pbf) {
    osmium::io::File f {"test.osh.pbf"};
    BOOST_CHECK_EQUAL(osmium::io::file_format::pbf, f.format());
    BOOST_CHECK_EQUAL(osmium::io::file_compression::none, f.compression());
    BOOST_CHECK_EQUAL(true, f.has_multiple_object_versions());
    f.check();
}

BOOST_AUTO_TEST_CASE(override_file_format_by_suffix_osm) {
    osmium::io::File f {"test", "osm"};
    BOOST_CHECK_EQUAL(osmium::io::file_format::xml, f.format());
    BOOST_CHECK_EQUAL(osmium::io::file_compression::none, f.compression());
    BOOST_CHECK_EQUAL(false, f.has_multiple_object_versions());
    f.check();
}

BOOST_AUTO_TEST_CASE(override_file_format_by_suffix_pbf) {
    osmium::io::File f {"test", "pbf"};
    BOOST_CHECK_EQUAL(osmium::io::file_format::pbf, f.format());
    BOOST_CHECK_EQUAL(osmium::io::file_compression::none, f.compression());
    BOOST_CHECK_EQUAL(false, f.has_multiple_object_versions());
    f.check();
}

BOOST_AUTO_TEST_CASE(override_file_format_by_suffix_osm_pbf) {
    osmium::io::File f {"test", "osm.pbf"};
    BOOST_CHECK_EQUAL(osmium::io::file_format::pbf, f.format());
    BOOST_CHECK_EQUAL(osmium::io::file_compression::none, f.compression());
    BOOST_CHECK_EQUAL(false, f.has_multiple_object_versions());
    f.check();
}

BOOST_AUTO_TEST_CASE(override_file_format_by_suffix_opl) {
    osmium::io::File f {"test", "opl"};
    BOOST_CHECK_EQUAL(osmium::io::file_format::opl, f.format());
    BOOST_CHECK_EQUAL(osmium::io::file_compression::none, f.compression());
    BOOST_CHECK_EQUAL(false, f.has_multiple_object_versions());
    f.check();
}

BOOST_AUTO_TEST_CASE(override_file_format_by_suffix_osm_opl) {
    osmium::io::File f {"test", "osm.opl"};
    BOOST_CHECK_EQUAL(osmium::io::file_format::opl, f.format());
    BOOST_CHECK_EQUAL(osmium::io::file_compression::none, f.compression());
    BOOST_CHECK_EQUAL(false, f.has_multiple_object_versions());
    f.check();
}

BOOST_AUTO_TEST_CASE(override_file_format_by_suffix_osm_gz) {
    osmium::io::File f {"test", "osm.gz"};
    BOOST_CHECK_EQUAL(osmium::io::file_format::xml, f.format());
    BOOST_CHECK_EQUAL(osmium::io::file_compression::gzip, f.compression());
    BOOST_CHECK_EQUAL(false, f.has_multiple_object_versions());
    f.check();
}

BOOST_AUTO_TEST_CASE(override_file_format_by_suffix_osm_opl_bz2) {
    osmium::io::File f {"test", "osm.opl.bz2"};
    BOOST_CHECK_EQUAL(osmium::io::file_format::opl, f.format());
    BOOST_CHECK_EQUAL(osmium::io::file_compression::bzip2, f.compression());
    BOOST_CHECK_EQUAL(false, f.has_multiple_object_versions());
    f.check();
}

BOOST_AUTO_TEST_CASE(override_file_format_by_suffix_osc_gz) {
    osmium::io::File f {"test", "osc.gz"};
    BOOST_CHECK_EQUAL(osmium::io::file_format::xml, f.format());
    BOOST_CHECK_EQUAL(osmium::io::file_compression::gzip, f.compression());
    BOOST_CHECK_EQUAL(true, f.has_multiple_object_versions());
    f.check();
}

BOOST_AUTO_TEST_CASE(override_file_format_by_suffix_osh_opl_gz) {
    osmium::io::File f {"test", "osh.opl.gz"};
    BOOST_CHECK_EQUAL(osmium::io::file_format::opl, f.format());
    BOOST_CHECK_EQUAL(osmium::io::file_compression::gzip, f.compression());
    BOOST_CHECK_EQUAL(true, f.has_multiple_object_versions());
    f.check();
}

BOOST_AUTO_TEST_CASE(override_file_format_by_suffix_osh_pbf) {
    osmium::io::File f {"test", "osh.pbf"};
    BOOST_CHECK_EQUAL(osmium::io::file_format::pbf, f.format());
    BOOST_CHECK_EQUAL(osmium::io::file_compression::none, f.compression());
    BOOST_CHECK_EQUAL(true, f.has_multiple_object_versions());
    f.check();
}

BOOST_AUTO_TEST_CASE(format_options_pbf_history) {
    osmium::io::File f {"test", "pbf,history=true"};
    BOOST_CHECK_EQUAL(osmium::io::file_format::pbf, f.format());
    BOOST_CHECK_EQUAL(osmium::io::file_compression::none, f.compression());
    BOOST_CHECK_EQUAL(true, f.has_multiple_object_versions());
    f.check();
}

BOOST_AUTO_TEST_CASE(format_options_pbf_foo) {
    osmium::io::File f {"test.osm", "pbf,foo=bar"};
    BOOST_CHECK_EQUAL(osmium::io::file_format::pbf, f.format());
    BOOST_CHECK_EQUAL(osmium::io::file_compression::none, f.compression());
    BOOST_CHECK_EQUAL("bar", f.get("foo"));
    f.check();
}

BOOST_AUTO_TEST_CASE(format_options_xml_abc_something) {
    osmium::io::File f {"test.bla", "xml,abc,some=thing"};
    BOOST_CHECK_EQUAL(osmium::io::file_format::xml, f.format());
    BOOST_CHECK_EQUAL(osmium::io::file_compression::none, f.compression());
    BOOST_CHECK_EQUAL("true", f.get("abc"));
    BOOST_CHECK_EQUAL("thing", f.get("some"));
    BOOST_CHECK_EQUAL(2, std::distance(f.begin(), f.end()));
    f.check();
}

BOOST_AUTO_TEST_CASE(unknown_format_foo_bar) {
    osmium::io::File f {"test.foo.bar"};
    BOOST_CHECK_EQUAL(osmium::io::file_format::unknown, f.format());
    BOOST_CHECK_EQUAL(osmium::io::file_compression::none, f.compression());
    BOOST_CHECK_THROW(f.check(), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(unknown_format_foo) {
    osmium::io::File f {"test", "foo"};
    BOOST_CHECK_THROW(f.check(), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(unknown_format_osm_foo) {
    osmium::io::File f {"test", "osm.foo"};
    BOOST_CHECK_THROW(f.check(), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(unknown_format_bla_equals_foo) {
    osmium::io::File f {"test", "bla=foo"};
    BOOST_CHECK_THROW(f.check(), std::runtime_error);
}

BOOST_AUTO_TEST_SUITE_END()

