#ifdef STAND_ALONE
# define BOOST_TEST_MODULE Main
#endif
#include <boost/test/unit_test.hpp>
#include <boost/test/output_test_stream.hpp>
using boost::test_tools::output_test_stream;

#include <type_traits>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <osmium/osm/types.hpp>
#include <osmium/osm/location.hpp>
#include <osmium/osm/ostream.hpp>

#include <osmium/index/map/dummy.hpp>
#include <osmium/index/map/sparse_table.hpp>
#include <osmium/index/map/stl_map.hpp>
#include <osmium/index/map/stl_vector.hpp>
#include <osmium/index/map/mmap_vector_anon.hpp>
#include <osmium/index/map/mmap_vector_file.hpp>

BOOST_AUTO_TEST_SUITE(IdToLocation)

template <typename TIndex>
void test_func_all(TIndex& index) {
    osmium::object_id_type id1 = 12;
    osmium::object_id_type id2 = 3;
    osmium::Location loc1(1.2, 4.5);
    osmium::Location loc2(3.5, -7.2);

    BOOST_CHECK_THROW(index.get(id1), std::out_of_range);

    index.set(id1, loc1);
    index.set(id2, loc2);

    index.sort();

    BOOST_CHECK_THROW(index.get(5), std::out_of_range);
    BOOST_CHECK_THROW(index.get(100), std::out_of_range);
}

template <typename TIndex>
void test_func_real(TIndex& index) {
    osmium::object_id_type id1 = 12;
    osmium::object_id_type id2 = 3;
    osmium::Location loc1(1.2, 4.5);
    osmium::Location loc2(3.5, -7.2);

    index.set(id1, loc1);
    index.set(id2, loc2);

    index.sort();

    BOOST_CHECK_EQUAL(loc1, index.get(id1));
    BOOST_CHECK_EQUAL(loc2, index.get(id2));

    BOOST_CHECK_THROW(index.get(5), std::out_of_range);
    BOOST_CHECK_THROW(index.get(100), std::out_of_range);

    index.clear();

    BOOST_CHECK_THROW(index.get(id1), std::out_of_range);
}

BOOST_AUTO_TEST_CASE(Dummy) {
    typedef osmium::index::map::Dummy<osmium::unsigned_object_id_type, osmium::Location> index_type;

    index_type index1;

    BOOST_CHECK_EQUAL(0, index1.size());
    BOOST_CHECK_EQUAL(0, index1.used_memory());

    test_func_all<index_type>(index1);

    BOOST_CHECK_EQUAL(0, index1.size());
    BOOST_CHECK_EQUAL(0, index1.used_memory());
}

BOOST_AUTO_TEST_CASE(DenseMapMem) {
    typedef osmium::index::map::DenseMapMem<osmium::unsigned_object_id_type, osmium::Location> index_type;

    index_type index1;
    index1.reserve(1000);
    test_func_all<index_type>(index1);

    index_type index2;
    index2.reserve(1000);
    test_func_real<index_type>(index2);
}

BOOST_AUTO_TEST_CASE(DenseMapMmap) {
    typedef osmium::index::map::DenseMapMmap<osmium::unsigned_object_id_type, osmium::Location> index_type;

    index_type index1;
    test_func_all<index_type>(index1);

    index_type index2;
    test_func_real<index_type>(index2);
}

BOOST_AUTO_TEST_CASE(DenseMapFile) {
    typedef osmium::index::map::DenseMapFile<osmium::unsigned_object_id_type, osmium::Location> index_type;

    index_type index1;
    test_func_all<index_type>(index1);

    index_type index2;
    test_func_real<index_type>(index2);
}

BOOST_AUTO_TEST_CASE(SparseTable) {
    typedef osmium::index::map::SparseTable<osmium::unsigned_object_id_type, osmium::Location> index_type;

    index_type index1;
    test_func_all<index_type>(index1);

    index_type index2;
    test_func_real<index_type>(index2);
}

BOOST_AUTO_TEST_CASE(StlMap) {
    typedef osmium::index::map::StlMap<osmium::unsigned_object_id_type, osmium::Location> index_type;

    index_type index1;
    test_func_all<index_type>(index1);

    index_type index2;
    test_func_real<index_type>(index2);
}

BOOST_AUTO_TEST_CASE(SparseMapMem) {
    typedef osmium::index::map::SparseMapMem<osmium::unsigned_object_id_type, osmium::Location> index_type;

    index_type index1;

    BOOST_CHECK_EQUAL(0, index1.size());
    BOOST_CHECK_EQUAL(0, index1.used_memory());

    test_func_all<index_type>(index1);

    BOOST_CHECK_EQUAL(2, index1.size());

    index_type index2;
    test_func_real<index_type>(index2);
}

BOOST_AUTO_TEST_SUITE_END()

