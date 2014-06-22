#ifdef STAND_ALONE
# define BOOST_TEST_MODULE Main
#endif
#include <boost/test/unit_test.hpp>
#include <boost/test/output_test_stream.hpp>
using boost::test_tools::output_test_stream;

#include <osmium/osm/box.hpp>
#include <osmium/geom/relations.hpp>

BOOST_AUTO_TEST_SUITE(Box)

BOOST_AUTO_TEST_CASE(instantiation) {
    osmium::Box b;
    BOOST_CHECK(!b);
    BOOST_CHECK(!b.bottom_left());
    BOOST_CHECK(!b.top_right());
    BOOST_CHECK_THROW(b.size(), osmium::invalid_location);
}

BOOST_AUTO_TEST_CASE(instantiation_and_extend_with_undefined) {
    osmium::Box b;
    BOOST_CHECK(!b);
    b.extend(osmium::Location());
    BOOST_CHECK(!b.bottom_left());
    BOOST_CHECK(!b.top_right());
}

BOOST_AUTO_TEST_CASE(instantiation_and_extend) {
    osmium::Box b;
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
    osmium::Box b;
    b.extend(osmium::Location(1.2, 3.4));
    b.extend(osmium::Location(5.6, 7.8));
    output_test_stream out;
    out << b;
    BOOST_CHECK(out.is_equal("(1.2,3.4,5.6,7.8)"));
    BOOST_CHECK_EQUAL(19.36, b.size());
}

BOOST_AUTO_TEST_CASE(output_undefined) {
    osmium::Box b;
    output_test_stream out;
    out << b;
    BOOST_CHECK(out.is_equal("(undefined)"));
}

BOOST_AUTO_TEST_CASE(box_inside_box) {
    osmium::Box outer;
    outer.extend(osmium::Location(1, 1));
    outer.extend(osmium::Location(10, 10));

    osmium::Box inner;
    inner.extend(osmium::Location(2, 2));
    inner.extend(osmium::Location(4, 4));

    osmium::Box overlap;
    overlap.extend(osmium::Location(3, 3));
    overlap.extend(osmium::Location(5, 5));

    BOOST_CHECK( osmium::geom::contains(inner, outer));
    BOOST_CHECK(!osmium::geom::contains(outer, inner));

    BOOST_CHECK(!osmium::geom::contains(overlap, inner));
    BOOST_CHECK(!osmium::geom::contains(inner, overlap));
}

BOOST_AUTO_TEST_SUITE_END()

