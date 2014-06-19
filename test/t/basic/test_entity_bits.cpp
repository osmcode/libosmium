#ifdef STAND_ALONE
# define BOOST_TEST_MODULE Main
#endif
#include <boost/test/unit_test.hpp>
#include <boost/test/output_test_stream.hpp>
using boost::test_tools::output_test_stream;

#include <osmium/osm/entity_bits.hpp>

BOOST_AUTO_TEST_SUITE(entity_bits)

BOOST_AUTO_TEST_CASE(can_be_set_and_checked) {
    osmium::osm_entity_bits::type entities = osmium::osm_entity_bits::node | osmium::osm_entity_bits::way;
    BOOST_CHECK(entities == osmium::osm_entity_bits::node | osmium::osm_entity_bits::way);

    entities |= osmium::osm_entity_bits::relation;
    BOOST_CHECK(entities & osmium::osm_entity_bits::object);

    entities |= osmium::osm_entity_bits::area;
    BOOST_CHECK(entities == osmium::osm_entity_bits::object);

    BOOST_CHECK(! (entities & osmium::osm_entity_bits::changeset));

    entities &= osmium::osm_entity_bits::node;
    BOOST_CHECK(entities & osmium::osm_entity_bits::node);
    BOOST_CHECK(! (entities & osmium::osm_entity_bits::way));
    BOOST_CHECK(entities == osmium::osm_entity_bits::node);
}

BOOST_AUTO_TEST_SUITE_END()
