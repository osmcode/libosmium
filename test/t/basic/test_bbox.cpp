#ifdef STAND_ALONE
# define BOOST_TEST_MODULE Main
#endif
#include <boost/test/unit_test.hpp>
#include <boost/test/output_test_stream.hpp>
using boost::test_tools::output_test_stream;

#include <osmium/osm/bbox.hpp>
#include <osmium/osm/ostream.hpp>

BOOST_AUTO_TEST_SUITE(BBox)

BOOST_AUTO_TEST_CASE(instantiation) {
    osmium::BBox b;
    BOOST_CHECK(!b);
    BOOST_CHECK(!b.bottom_left());
    BOOST_CHECK(!b.top_right());
}

BOOST_AUTO_TEST_CASE(instantiation_and_extend_with_undefined) {
    osmium::BBox b;
    BOOST_CHECK(!b);
    BOOST_CHECK(!b.bottom_left());
    BOOST_CHECK(!b.top_right());
}

BOOST_AUTO_TEST_CASE(instantiation_and_extend) {
    osmium::BBox b;
    b.extend(osmium::Location(1.2, 3.4));
    BOOST_CHECK(!!b);
    BOOST_CHECK(!!b.bottom_left());
    BOOST_CHECK(!!b.top_right());
    b.extend(osmium::Location(3.4, 4.5));
    b.extend(osmium::Location(5.6, 7.8));
    BOOST_CHECK_EQUAL(b.bottom_left(), osmium::Location(1.2, 3.4));
    BOOST_CHECK_EQUAL(b.top_right(), osmium::Location(5.6, 7.8));

    // extend with undefined doesn't change anything
    b.extend(osmium::Location());
    BOOST_CHECK_EQUAL(b.bottom_left(), osmium::Location(1.2, 3.4));
    BOOST_CHECK_EQUAL(b.top_right(), osmium::Location(5.6, 7.8));
}

BOOST_AUTO_TEST_CASE(output_defined) {
    osmium::BBox b;
    b.extend(osmium::Location(1.2, 3.4));
    b.extend(osmium::Location(5.6, 7.8));
    output_test_stream out;
    out << b;
    BOOST_CHECK(out.is_equal("(1.2,3.4,5.6,7.8)"));
}

BOOST_AUTO_TEST_CASE(output_undefined) {
    osmium::BBox b;
    output_test_stream out;
    out << b;
    BOOST_CHECK(out.is_equal("(undefined)"));
}

BOOST_AUTO_TEST_SUITE_END()

