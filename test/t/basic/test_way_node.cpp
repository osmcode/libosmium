#ifdef STAND_ALONE
# define BOOST_TEST_MODULE Main
#endif
#include <boost/test/unit_test.hpp>

#include <osmium/osm/way.hpp>
#include <osmium/osm/ostream.hpp>

BOOST_AUTO_TEST_SUITE(NodeRef)

BOOST_AUTO_TEST_CASE(instantiation_with_default_parameters) {
    osmium::NodeRef node_ref;
    BOOST_CHECK_EQUAL(node_ref.ref(), 0);
//    BOOST_CHECK(!node_ref.has_location());
}

BOOST_AUTO_TEST_CASE(instantiation_with_id) {
    osmium::NodeRef node_ref(7);
    BOOST_CHECK_EQUAL(node_ref.ref(), 7);
}

BOOST_AUTO_TEST_CASE(equality) {
    osmium::NodeRef node_ref1(7);
    osmium::NodeRef node_ref2(7);
    osmium::NodeRef node_ref3(9);
    BOOST_CHECK(node_ref1 == node_ref2);
    BOOST_CHECK(node_ref1 != node_ref3);
}

BOOST_AUTO_TEST_CASE(set_location) {
    osmium::NodeRef node_ref(7);
    BOOST_CHECK_EQUAL(node_ref.location(), osmium::Location());
    node_ref.location(osmium::Location(13.5, -7.2));
    BOOST_CHECK_EQUAL(node_ref.location().lon(), 13.5);
//    BOOST_CHECK(node_ref.has_location());
}

BOOST_AUTO_TEST_SUITE_END()

