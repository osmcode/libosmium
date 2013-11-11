#ifdef STAND_ALONE
# define BOOST_TEST_MODULE Main
#endif
#include <boost/test/unit_test.hpp>

#include <osmium/osm/way.hpp>
#include <osmium/osm/ostream.hpp>

BOOST_AUTO_TEST_SUITE(WayNode)

BOOST_AUTO_TEST_CASE(instantiation_with_default_parameters) {
    osmium::WayNode wn;
    BOOST_CHECK_EQUAL(wn.ref(), 0);
//    BOOST_CHECK(!wn.has_location());
}

BOOST_AUTO_TEST_CASE(instantiation_with_id) {
    osmium::WayNode wn(7);
    BOOST_CHECK_EQUAL(wn.ref(), 7);
}

BOOST_AUTO_TEST_CASE(equality) {
    osmium::WayNode wn1(7);
    osmium::WayNode wn2(7);
    osmium::WayNode wn3(9);
    BOOST_CHECK(wn1 == wn2);
    BOOST_CHECK(wn1 != wn3);
}

BOOST_AUTO_TEST_CASE(set_location) {
    osmium::WayNode wn(7);
    BOOST_CHECK_EQUAL(wn.location(), osmium::Location());
    wn.location(osmium::Location(13.5, -7.2));
    BOOST_CHECK_EQUAL(wn.location().lon(), 13.5);
//    BOOST_CHECK(wn.has_location());
}

BOOST_AUTO_TEST_SUITE_END()

