#ifdef STAND_ALONE
# define BOOST_TEST_MODULE Main
#endif
#include <boost/test/unit_test.hpp>
#include <boost/test/output_test_stream.hpp>
using boost::test_tools::output_test_stream;

#include <iterator>

#include <osmium/util/options.hpp>

BOOST_AUTO_TEST_SUITE(Options)

BOOST_AUTO_TEST_CASE(set_simple) {
    osmium::util::Options o;
    o.set("foo", "bar");
    BOOST_CHECK_EQUAL("bar", o.get("foo"));
    BOOST_CHECK_EQUAL("", o.get("empty"));
    BOOST_CHECK_EQUAL("default", o.get("empty", "default"));
    BOOST_CHECK(!o.is_true("foo"));
    BOOST_CHECK(!o.is_true("empty"));
    BOOST_CHECK_EQUAL(1, o.size());
}

BOOST_AUTO_TEST_CASE(set_from_bool) {
    osmium::util::Options o;
    o.set("t", true);
    o.set("f", false);
    BOOST_CHECK_EQUAL("true", o.get("t"));
    BOOST_CHECK_EQUAL("false", o.get("f"));
    BOOST_CHECK_EQUAL("", o.get("empty"));
    BOOST_CHECK(o.is_true("t"));
    BOOST_CHECK(!o.is_true("f"));
    BOOST_CHECK_EQUAL(2, o.size());
}

BOOST_AUTO_TEST_CASE(set_from_single_string_with_equals) {
    osmium::util::Options o;
    o.set("foo=bar");
    BOOST_CHECK_EQUAL("bar", o.get("foo"));
    BOOST_CHECK_EQUAL(1, o.size());
}

BOOST_AUTO_TEST_CASE(set_from_single_string_without_equals) {
    osmium::util::Options o;
    o.set("foo");
    BOOST_CHECK_EQUAL("true", o.get("foo"));
    BOOST_CHECK(o.is_true("foo"));
    BOOST_CHECK_EQUAL(1, o.size());
}

BOOST_AUTO_TEST_SUITE_END()

