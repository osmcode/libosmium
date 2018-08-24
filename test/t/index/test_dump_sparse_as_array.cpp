#include "catch.hpp"

#include <osmium/index/detail/tmpfile.hpp>
#include <osmium/index/map/dense_file_array.hpp>
#include <osmium/index/map/sparse_mmap_array.hpp>
#include <osmium/index/node_locations_map.hpp>
#include <osmium/osm/location.hpp>
#include <osmium/osm/types.hpp>
#include <osmium/util/file.hpp>

#include <iterator>

using sparse_mmap_array = osmium::index::map::SparseMmapArray<osmium::unsigned_object_id_type, osmium::Location>;
using dense_file_array = osmium::index::map::DenseFileArray<osmium::unsigned_object_id_type, osmium::Location>;

TEST_CASE("Dump SparseMmapArray as array and load it as DenseFileArray") {

    const int fd = osmium::detail::create_tmp_file();
    const constexpr size_t BUFFER_SIZE = 10L * 1024L * 1024L / sizeof(sparse_mmap_array::element_type::second_type);

    REQUIRE(osmium::file_size(fd) == 0);

    const osmium::unsigned_object_id_type id1 = 1;
    const osmium::unsigned_object_id_type id2 = 6;
    const osmium::unsigned_object_id_type id3 = 2 * BUFFER_SIZE;
    const osmium::unsigned_object_id_type id4 = id3 + 1;
    const osmium::unsigned_object_id_type id5 = 3 * BUFFER_SIZE - 1;
    const osmium::unsigned_object_id_type id6 = 3 * BUFFER_SIZE;
    const osmium::Location loc1{1.2, 4.5};
    const osmium::Location loc2{3.5, -7.2};
    const osmium::Location loc3{10.2, 64.5};
    const osmium::Location loc4{39.5, -71.2};
    const osmium::Location loc5{-1.2, 54.6};
    const osmium::Location loc6{-9.3, 171.2};

    constexpr const size_t S = sizeof(dense_file_array::element_type);
    sparse_mmap_array sparse_index;
    sparse_index.set(id1, loc1);
    sparse_index.set(id2, loc2);
    sparse_index.set(id3, loc3);
    sparse_index.set(id4, loc4);
    sparse_index.set(id5, loc5);
    sparse_index.set(id6, loc6);
    sparse_index.sort();
    sparse_index.dump_as_array(fd);

    dense_file_array dense_index{fd};
    REQUIRE(osmium::file_size(fd) >= (id6 * S));
    REQUIRE_THROWS_AS(dense_index.get(0), const osmium::not_found&);
    REQUIRE_THROWS_AS(dense_index.get((BUFFER_SIZE - 1) / sizeof(osmium::Location)), const osmium::not_found&);
    REQUIRE_THROWS_AS(dense_index.get(BUFFER_SIZE / sizeof(osmium::Location)), const osmium::not_found&);
    REQUIRE_THROWS_AS(dense_index.get(id2 - 1), const osmium::not_found&);
    REQUIRE_THROWS_AS(dense_index.get(id2 + 1), const osmium::not_found&);
    REQUIRE_THROWS_AS(dense_index.get(100), const osmium::not_found&);
    REQUIRE_THROWS_AS(dense_index.get(id3 - 1), const osmium::not_found&);
    REQUIRE_THROWS_AS(dense_index.get(BUFFER_SIZE) / sizeof(osmium::Location), const osmium::not_found&);
    REQUIRE_THROWS_AS(dense_index.get(4 * BUFFER_SIZE), const osmium::not_found&);
    REQUIRE_THROWS_AS(dense_index.get(4 * BUFFER_SIZE - 1), const osmium::not_found&);

    REQUIRE(dense_index.get(id1) == loc1);
    REQUIRE(dense_index.get(id2) == loc2);
    REQUIRE(dense_index.get(id3) == loc3);
    REQUIRE(dense_index.get(id4) == loc4);
    REQUIRE(dense_index.get(id5) == loc5);
    REQUIRE(dense_index.get(id6) == loc6);
}
