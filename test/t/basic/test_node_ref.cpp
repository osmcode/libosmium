#ifdef STAND_ALONE
# define BOOST_TEST_MODULE Main
#endif
#include <boost/test/unit_test.hpp>

#include <osmium/osm/node_ref.hpp>

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
    osmium::NodeRef node_ref1(7, { 1.2, 3.4 });
    osmium::NodeRef node_ref2(7, { 1.4, 3.1 });
    osmium::NodeRef node_ref3(9, { 1.2, 3.4 });
    BOOST_CHECK_EQUAL(node_ref1, node_ref2);
    BOOST_CHECK_NE(node_ref1, node_ref3);
    BOOST_CHECK(!osmium::location_equal()(node_ref1, node_ref2));
    BOOST_CHECK(!osmium::location_equal()(node_ref2, node_ref3));
    BOOST_CHECK(osmium::location_equal()(node_ref1, node_ref3));
}

BOOST_AUTO_TEST_CASE(set_location) {
    osmium::NodeRef node_ref(7);
    BOOST_CHECK_EQUAL(node_ref.location(), osmium::Location());
    node_ref.location(osmium::Location(13.5, -7.2));
    BOOST_CHECK_EQUAL(node_ref.location().lon(), 13.5);
//    BOOST_CHECK(node_ref.has_location());
}

BOOST_AUTO_TEST_CASE(ordering) {
    osmium::NodeRef node_ref1(1, { 1.0, 3.0 });
    osmium::NodeRef node_ref2(2, { 1.4, 2.9 });
    osmium::NodeRef node_ref3(3, { 1.2, 3.0 });
    osmium::NodeRef node_ref4(4, { 1.2, 3.3 });

    BOOST_CHECK_LT(node_ref1, node_ref2);
    BOOST_CHECK_LT(node_ref2, node_ref3);
    BOOST_CHECK_LT(node_ref1, node_ref3);
    BOOST_CHECK_GE(node_ref1, node_ref1);

    BOOST_CHECK(osmium::location_less()(node_ref1, node_ref2));
    BOOST_CHECK(!osmium::location_less()(node_ref2, node_ref3));
    BOOST_CHECK(osmium::location_less()(node_ref1, node_ref3));
    BOOST_CHECK(osmium::location_less()(node_ref3, node_ref4));
    BOOST_CHECK(!osmium::location_less()(node_ref1, node_ref1));
}

BOOST_AUTO_TEST_SUITE_END()

