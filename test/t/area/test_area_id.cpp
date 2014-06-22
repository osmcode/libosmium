#ifdef STAND_ALONE
# define BOOST_TEST_MODULE Main
#endif
#include <boost/test/unit_test.hpp>

#include <osmium/osm/area.hpp>

BOOST_AUTO_TEST_SUITE(area_id)

BOOST_AUTO_TEST_CASE(object_id_to_area_id_conversion) {
    BOOST_CHECK_EQUAL( 46, osmium::object_id_to_area_id( 23, osmium::item_type::way));
    BOOST_CHECK_EQUAL( 47, osmium::object_id_to_area_id( 23, osmium::item_type::relation));
    BOOST_CHECK_EQUAL(  0, osmium::object_id_to_area_id(  0, osmium::item_type::way));
    BOOST_CHECK_EQUAL(  1, osmium::object_id_to_area_id(  0, osmium::item_type::relation));
    BOOST_CHECK_EQUAL(-24, osmium::object_id_to_area_id(-12, osmium::item_type::way));
    BOOST_CHECK_EQUAL(-25, osmium::object_id_to_area_id(-12, osmium::item_type::relation));
}

BOOST_AUTO_TEST_CASE(area_id_to_object_id_conversion) {
    BOOST_CHECK_EQUAL( 23, osmium::area_id_to_object_id( 46));
    BOOST_CHECK_EQUAL( 23, osmium::area_id_to_object_id( 47));
    BOOST_CHECK_EQUAL(  0, osmium::area_id_to_object_id(  0));
    BOOST_CHECK_EQUAL(  0, osmium::area_id_to_object_id(  1));
    BOOST_CHECK_EQUAL(-12, osmium::area_id_to_object_id(-24));
    BOOST_CHECK_EQUAL(-12, osmium::area_id_to_object_id(-25));
}

BOOST_AUTO_TEST_SUITE_END()
