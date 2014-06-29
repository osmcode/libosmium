
#ifdef STAND_ALONE
# define BOOST_TEST_MODULE Main
#endif
#include <boost/test/unit_test.hpp>

#include <sstream>

#include <osmium/osm/timestamp.hpp>

BOOST_AUTO_TEST_SUITE(Timestamp)

BOOST_AUTO_TEST_CASE(can_be_default_initialized) {
    osmium::Timestamp t;
    BOOST_CHECK_EQUAL(0, t);
    BOOST_CHECK_EQUAL("", t.to_iso());
}

BOOST_AUTO_TEST_CASE(invalid_value) {
    osmium::Timestamp t(static_cast<time_t>(0));
    BOOST_CHECK_EQUAL(0, t);
    BOOST_CHECK_EQUAL("", t.to_iso());
}

BOOST_AUTO_TEST_CASE(can_be_initialized_from_time_t) {
    osmium::Timestamp t(static_cast<time_t>(1));
    BOOST_CHECK_EQUAL(1, t);
    BOOST_CHECK_EQUAL("1970-01-01T00:00:01Z", t.to_iso());
}

BOOST_AUTO_TEST_CASE(can_be_compared) {
    osmium::Timestamp t1(10);
    osmium::Timestamp t2(50);
    BOOST_CHECK_LT(t1, t2);
}

BOOST_AUTO_TEST_CASE(can_be_written_to_stream) {
    std::stringstream ss;
    osmium::Timestamp t(1);
    ss << t;
    BOOST_CHECK_EQUAL("1970-01-01T00:00:01Z", ss.str());
}

BOOST_AUTO_TEST_SUITE_END()
