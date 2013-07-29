#ifdef STAND_ALONE
# define BOOST_TEST_MODULE Main
#endif
#include <boost/test/unit_test.hpp>

#include <osmium/osm/relation.hpp>

BOOST_AUTO_TEST_SUITE(Basic_Relation)

BOOST_AUTO_TEST_CASE(relation) {
    osmium::Relation relation;

    relation.id(17);
    relation.version(3);
    relation.visible(true);
    relation.changeset(333);
    relation.uid(21);
    relation.timestamp(123);

    BOOST_CHECK_EQUAL(17, relation.id());
    BOOST_CHECK_EQUAL(3, relation.version());
    BOOST_CHECK_EQUAL(true, relation.visible());
    BOOST_CHECK_EQUAL(333, relation.changeset());
    BOOST_CHECK_EQUAL(21, relation.uid());
    BOOST_CHECK_EQUAL(123, relation.timestamp());
}

BOOST_AUTO_TEST_SUITE_END()
