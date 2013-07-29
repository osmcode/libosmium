#ifdef STAND_ALONE
# define BOOST_TEST_MODULE Main
#endif
#include <boost/test/unit_test.hpp>

#include <osmium/osm/way.hpp>

BOOST_AUTO_TEST_SUITE(Basic_Way)

BOOST_AUTO_TEST_CASE(way) {
    osmium::Way way;

    way.id(17);
    way.version(3);
    way.visible(true);
    way.changeset(333);
    way.uid(21);
    way.timestamp(123);

    BOOST_CHECK_EQUAL(17, way.id());
    BOOST_CHECK_EQUAL(3, way.version());
    BOOST_CHECK_EQUAL(true, way.visible());
    BOOST_CHECK_EQUAL(333, way.changeset());
    BOOST_CHECK_EQUAL(21, way.uid());
    BOOST_CHECK_EQUAL(123, way.timestamp());
}

BOOST_AUTO_TEST_SUITE_END()
