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
}

BOOST_AUTO_TEST_CASE(instantiation_with_double_parameters_constructor_with_universal_initializer) {
    osmium::Location loc { 2.2, 3.3 };
    BOOST_CHECK_EQUAL(2.2, loc.lon());
    BOOST_CHECK_EQUAL(3.3, loc.lat());
}

BOOST_AUTO_TEST_CASE(instantiation_with_double_parameters_constructor_with_initializer_list) {
    osmium::Location loc({ 4.4, 5.5 });
    BOOST_CHECK_EQUAL(4.4, loc.lon());
    BOOST_CHECK_EQUAL(5.5, loc.lat());
}

BOOST_AUTO_TEST_CASE(instantiation_with_double_parameters_operator_equal) {
    osmium::Location loc = { 5.5, 6.6 };
    BOOST_CHECK_EQUAL(5.5, loc.lon());
    BOOST_CHECK_EQUAL(6.6, loc.lat());
}

BOOST_AUTO_TEST_CASE(equality) {
    osmium::Location loc1(1.2, 4.5);
    osmium::Location loc2(1.2, 4.5);
    osmium::Location loc3(1.5, 1.5);
    BOOST_CHECK_EQUAL(loc1, loc2);
    BOOST_CHECK(loc1 != loc3);
}

BOOST_AUTO_TEST_CASE(order) {
    BOOST_CHECK(osmium::Location(-1.2, 10.0) < osmium::Location(1.2, 10.0));
    BOOST_CHECK(osmium::Location(1.2, 10.0) > osmium::Location(-1.2, 10.0));

    BOOST_CHECK(osmium::Location(10.2, 20.0) < osmium::Location(11.2, 20.2));
    BOOST_CHECK(osmium::Location(10.2, 20.2) < osmium::Location(11.2, 20.0));
    BOOST_CHECK(osmium::Location(11.2, 20.2) > osmium::Location(10.2, 20.0));
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


BOOST_AUTO_TEST_CASE(output_to_iterator_comma_separator) {
    char buffer[100];
    osmium::Location loc(-3.2, 47.3);
    *loc.as_string(buffer, ',') = 0;
    BOOST_CHECK(!strcmp(buffer, "-3.2,47.3"));
}

BOOST_AUTO_TEST_CASE(output_to_iterator_space_separator) {
    char buffer[100];
    osmium::Location loc(0.0, 7.0);
    *loc.as_string(buffer, ' ') = 0;
    BOOST_CHECK(!strcmp(buffer, "0 7"));
}

BOOST_AUTO_TEST_CASE(output_to_iterator_check_precision) {
    char buffer[100];
    osmium::Location loc(-179.9999999, -90.0);
    *loc.as_string(buffer, ' ') = 0;
    BOOST_CHECK(!strcmp(buffer, "-179.9999999 -90"));
}

BOOST_AUTO_TEST_CASE(output_to_iterator_undefined_location) {
    char buffer[100];
    osmium::Location loc;
    *loc.as_string(buffer, ',') = 0;
    BOOST_CHECK(!strcmp(buffer, "undefined,undefined"));
}

BOOST_AUTO_TEST_CASE(output_to_string_comman_separator) {
    std::string s;
    osmium::Location loc(-3.2, 47.3);
    loc.as_string(std::back_inserter(s), ',');
    BOOST_CHECK_EQUAL(s, "-3.2,47.3");
}

BOOST_AUTO_TEST_CASE(output_to_string_space_separator) {
    std::string s;
    osmium::Location loc(0.0, 7.0);
    loc.as_string(std::back_inserter(s), ' ');
    BOOST_CHECK_EQUAL(s, "0 7");
}

BOOST_AUTO_TEST_CASE(output_to_string_check_precision) {
    std::string s;
    osmium::Location loc(-179.9999999, -90.0);
    loc.as_string(std::back_inserter(s), ' ');
    BOOST_CHECK_EQUAL(s, "-179.9999999 -90");
}

BOOST_AUTO_TEST_CASE(output_to_string_undefined_location) {
    std::string s;
    osmium::Location loc;
    loc.as_string(std::back_inserter(s), ',');
    BOOST_CHECK_EQUAL(s, std::string("undefined,undefined"));
}

BOOST_AUTO_TEST_CASE(output_defined) {
    osmium::Location p(-3.2, 47.3);
    output_test_stream out;
    out << p;
    BOOST_CHECK(out.is_equal("(-3.2,47.3)"));
}

BOOST_AUTO_TEST_CASE(output_undefined) {
    osmium::Location p;
    output_test_stream out;
    out << p;
    BOOST_CHECK(out.is_equal("(undefined,undefined)"));
}

BOOST_AUTO_TEST_SUITE_END()

