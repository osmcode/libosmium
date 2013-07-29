#ifdef STAND_ALONE
# define BOOST_TEST_MODULE Main
#endif
#include <boost/test/unit_test.hpp>

#include <osmium/osm/object.hpp>

BOOST_AUTO_TEST_SUITE(Basic_Object)

BOOST_AUTO_TEST_CASE(object) {
    osmium::Object o;

    o.id(17);
    o.version(3);
    o.visible(true);
    o.changeset(333);
    o.uid(21);
    o.timestamp(123);

    BOOST_CHECK_EQUAL(17, o.id());
    BOOST_CHECK_EQUAL(3, o.version());
    BOOST_CHECK_EQUAL(true, o.visible());
    BOOST_CHECK_EQUAL(333, o.changeset());
    BOOST_CHECK_EQUAL(21, o.uid());
    BOOST_CHECK_EQUAL(123, o.timestamp());

    o.visible(false);
    BOOST_CHECK_EQUAL(3, o.version());
    BOOST_CHECK_EQUAL(false, o.visible());

    o.version(100);
    BOOST_CHECK_EQUAL(100, o.version());
    BOOST_CHECK_EQUAL(false, o.visible());
}

BOOST_AUTO_TEST_CASE(object_from_string) {
    osmium::Object o;

    o.id("17");
    o.version("3");
    o.visible(true);
    o.changeset("333");
    o.uid("21");

    BOOST_CHECK_EQUAL(17, o.id());
    BOOST_CHECK_EQUAL(3, o.version());
    BOOST_CHECK_EQUAL(true, o.visible());
    BOOST_CHECK_EQUAL(333, o.changeset());
    BOOST_CHECK_EQUAL(21, o.uid());
}

BOOST_AUTO_TEST_CASE(instantiation_with_default_parameters) {
    osmium::Object object;
    BOOST_CHECK_EQUAL(0, object.id());
    BOOST_CHECK_EQUAL(0, object.version());
    BOOST_CHECK_EQUAL(0, object.uid());
}

BOOST_AUTO_TEST_CASE(order) {
    osmium::Object object1;
    osmium::Object object2;
    object1.id(10);
    object1.version(1);
    object2.id(15);
    object2.version(2);
    BOOST_CHECK_EQUAL(true, object1 < object2);
    BOOST_CHECK_EQUAL(false, object1 > object2);
    object1.id(20);
    object1.version(1);
    object2.id(20);
    object2.version(2);
    BOOST_CHECK_EQUAL(true, object1 < object2);
    BOOST_CHECK_EQUAL(false, object1 > object2);
    object1.id(-10);
    object1.version(2);
    object2.id(-15);
    object2.version(1);
    BOOST_CHECK_EQUAL(true, object1 < object2);
    BOOST_CHECK_EQUAL(false, object1 > object2);
}

BOOST_AUTO_TEST_SUITE_END()
