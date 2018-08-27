#include "catch.hpp"

#include <osmium/index/detail/tmpfile.hpp>
#include <osmium/index/map/dense_file_array.hpp>
#include <osmium/index/map/dense_mmap_array.hpp>
#include <osmium/index/map/dense_mem_array.hpp>
#include <osmium/osm/location.hpp>
#include <osmium/osm/types.hpp>

using dense_mem_array = osmium::index::map::DenseMemArray<osmium::unsigned_object_id_type, osmium::Location>;
using dense_file_array = osmium::index::map::DenseFileArray<osmium::unsigned_object_id_type, osmium::Location>;

template <class TMemoryIndex>
void test_index() {
    const int fd = osmium::detail::create_tmp_file();
    REQUIRE(osmium::file_size(fd) == 0);
    constexpr const size_t ELEMENT_SIZE = sizeof(dense_file_array::element_type);
    const osmium::unsigned_object_id_type id1 = 12;
    const osmium::unsigned_object_id_type id2 = 3;
    const osmium::unsigned_object_id_type id3 = 7;
    const osmium::Location loc1{1.2, 4.5};
    const osmium::Location loc2{3.5, -7.2};
    const osmium::Location loc3{-12.7, 14.5};

    TMemoryIndex index;
    index.set(id1, loc1);
    index.set(id2, loc2);
    index.set(id3, loc3);

    // We don't sort the index because sorting is not required for DenseMmapArray.
    // dump to file
    index.dump_as_array(fd);

    REQUIRE(osmium::file_size(fd) >= (3 * ELEMENT_SIZE));

    // load index from file
    dense_file_array file_index {fd};

    // test retrievals
    REQUIRE(loc1 == file_index.get(id1));
    REQUIRE(loc2 == file_index.get(id2));
    REQUIRE(loc3 == file_index.get(id3));
    REQUIRE_THROWS_AS(file_index.get(5), const osmium::not_found&);
    REQUIRE_THROWS_AS(file_index.get(6), const osmium::not_found&);
    REQUIRE_THROWS_AS(file_index.get(200), const osmium::not_found&);
}

#ifdef __linux__
using dense_mmap_array = osmium::index::map::DenseMmapArray<osmium::unsigned_object_id_type, osmium::Location>;

TEST_CASE("Dump DenseMmapArray, load as DenseFileArray") {
    test_index<dense_mmap_array>();
}
#else
# pragma message("not running 'DenseMmapArray' test case on this machine")
#endif

TEST_CASE("Dump DenseMemArray, load as DenseFileArray") {
    test_index<dense_mem_array>();
}
