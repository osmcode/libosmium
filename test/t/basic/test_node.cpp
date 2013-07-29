#ifdef STAND_ALONE
# define BOOST_TEST_MODULE Main
#endif
#include <boost/test/unit_test.hpp>

#include <osmium/osm/node.hpp>
#include <osmium/osm/ostream.hpp>

BOOST_AUTO_TEST_SUITE(Basic_Node)

BOOST_AUTO_TEST_CASE(node) {
    osmium::Node node;

    node.id(17);
    node.version(3);
    node.visible(true);
    node.changeset(333);
    node.uid(21);
    node.timestamp(123);

    osmium::Location p(3.5, 4.7);
    node.location(p);

    BOOST_CHECK_EQUAL(17, node.id());
    BOOST_CHECK_EQUAL(3, node.version());
    BOOST_CHECK_EQUAL(true, node.visible());
    BOOST_CHECK_EQUAL(333, node.changeset());
    BOOST_CHECK_EQUAL(21, node.uid());
    BOOST_CHECK_EQUAL(123, node.timestamp());
    BOOST_CHECK_EQUAL(p, node.location());
}

BOOST_AUTO_TEST_SUITE_END()
