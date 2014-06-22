#ifdef STAND_ALONE
# define BOOST_TEST_MODULE Main
#endif
#include <boost/test/unit_test.hpp>

#include <osmium/osm/changeset.hpp>

#include "helper.hpp"

BOOST_AUTO_TEST_SUITE(Basic_Changeset)

BOOST_AUTO_TEST_CASE(changeset_builder) {
    osmium::memory::Buffer buffer(10 * 1000);

    osmium::Changeset& cs1 = buffer_add_changeset(buffer,
        "user",
        {{"comment", "foo"}});

    cs1.id(42)
       .created_at(100)
       .closed_at(200)
       .num_changes(7)
       .uid(9);

    BOOST_CHECK_EQUAL(42, cs1.id());
    BOOST_CHECK_EQUAL(9, cs1.uid());
    BOOST_CHECK_EQUAL(7, cs1.num_changes());
    BOOST_CHECK_EQUAL(true, cs1.closed());
    BOOST_CHECK_EQUAL(osmium::Timestamp(100), cs1.created_at());
    BOOST_CHECK_EQUAL(osmium::Timestamp(200), cs1.closed_at());
    BOOST_CHECK_EQUAL(1, cs1.tags().size());
    BOOST_CHECK_EQUAL(std::string("user"), cs1.user());

    osmium::Changeset& cs2 = buffer_add_changeset(buffer,
        "user",
        {{"comment", "foo"}, {"foo", "bar"}});

    cs2.id(43)
       .created_at(120)
       .num_changes(21)
       .uid(9);

    BOOST_CHECK_EQUAL(43, cs2.id());
    BOOST_CHECK_EQUAL(9, cs2.uid());
    BOOST_CHECK_EQUAL(21, cs2.num_changes());
    BOOST_CHECK_EQUAL(false, cs2.closed());
    BOOST_CHECK_EQUAL(osmium::Timestamp(120), cs2.created_at());
    BOOST_CHECK_EQUAL(osmium::Timestamp(), cs2.closed_at());
    BOOST_CHECK_EQUAL(2, cs2.tags().size());
    BOOST_CHECK_EQUAL(std::string("user"), cs2.user());

    BOOST_CHECK_EQUAL(false, cs1 == cs2);
    BOOST_CHECK_EQUAL(true, cs1 != cs2);
    BOOST_CHECK_EQUAL(true, cs1 == cs1);

    BOOST_CHECK_EQUAL(true, cs1 < cs2);
    BOOST_CHECK_EQUAL(true, cs1 <= cs2);
    BOOST_CHECK_EQUAL(false, cs1 > cs2);
    BOOST_CHECK_EQUAL(false, cs1 >= cs2);
}

BOOST_AUTO_TEST_SUITE_END()
