#ifdef STAND_ALONE
# define BOOST_TEST_MODULE Main
#endif
#include <boost/test/unit_test.hpp>
#include <boost/test/output_test_stream.hpp>
using boost::test_tools::output_test_stream;

#include <type_traits>

#include <osmium/osm/location.hpp>
#include <osmium/osm/ostream.hpp>

BOOST_AUTO_TEST_SUITE(Location)

static_assert(std::is_literal_type<osmium::Location>::value, "osmium::Location not literal type");

BOOST_AUTO_TEST_CASE(instantiation_with_default_parameters) {
    osmium::Location loc;
    BOOST_CHECK(!loc);
}

BOOST_AUTO_TEST_CASE(instantiation_with_double_parameters) {
    osmium::Location loc1(1.2, 4.5);
    BOOST_CHECK(!!loc1);
    BOOST_CHECK_EQUAL(12000000, loc1.x());
    BOOST_CHECK_EQUAL(45000000, loc1.y());
    BOOST_CHECK_EQUAL(1.2, loc1.lon());
    BOOST_CHECK_EQUAL(4.5, loc1.lat());

    osmium::Location loc2(loc1);
    BOOST_CHECK_EQUAL(4.5, loc2.lat());

    osmium::Location loc3 = loc1;
    BOOST_CHECK_EQUAL(4.5, loc3.lat());

    osmium::Location loc4 { 2.2, 3.3 };
    BOOST_CHECK_EQUAL(2.2, loc4.lon());
    BOOST_CHECK_EQUAL(3.3, loc4.lat());

    osmium::Location loc5({ 4.4, 5.5 });
    BOOST_CHECK_EQUAL(4.4, loc5.lon());
    BOOST_CHECK_EQUAL(5.5, loc5.lat());

    osmium::Location loc6 = { 5.5, 6.6 };
    BOOST_CHECK_EQUAL(5.5, loc6.lon());
    BOOST_CHECK_EQUAL(6.6, loc6.lat());
}

BOOST_AUTO_TEST_CASE(equality) {
    osmium::Location loc1(1.2, 4.5);
    osmium::Location loc2(1.2, 4.5);
    osmium::Location loc3(1.5, 1.5);
    BOOST_CHECK_EQUAL(loc1, loc2);
    BOOST_CHECK(loc1 != loc3);
}

BOOST_AUTO_TEST_CASE(validity) {
    BOOST_CHECK(osmium::Location(0.0, 0.0).valid());
    BOOST_CHECK(osmium::Location(1.2, 4.5).valid());
    BOOST_CHECK(osmium::Location(-1.2, 4.5).valid());
    BOOST_CHECK(osmium::Location(-180.0, -90.0).valid());
    BOOST_CHECK(osmium::Location(180.0, -90.0).valid());
    BOOST_CHECK(osmium::Location(-180.0, 90.0).valid());
    BOOST_CHECK(osmium::Location(180.0, 90.0).valid());

    BOOST_CHECK(!osmium::Location(200.0, 4.5).valid());
    BOOST_CHECK(!osmium::Location(-1.2, -100.0).valid());
    BOOST_CHECK(!osmium::Location(-180.0, 90.005).valid());
}

BOOST_AUTO_TEST_CASE(output) {
    osmium::Location p(-3.2, 47.3);
    output_test_stream out;
    out << p;
    BOOST_CHECK(out.is_equal("(-3.2,47.3)"));
}

BOOST_AUTO_TEST_SUITE_END()

