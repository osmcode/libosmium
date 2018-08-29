#include "catch.hpp"

#include <osmium/index/detail/tmpfile.hpp>
#include <osmium/index/map/dense_file_array.hpp>
#include <osmium/index/map/sparse_mmap_array.hpp>
#include <osmium/index/node_locations_map.hpp>
#include <osmium/osm/location.hpp>
#include <osmium/osm/types.hpp>
#include <osmium/util/file.hpp>

#include <vector>

using sparse_mmap_array = osmium::index::map::SparseMmapArray<osmium::unsigned_object_id_type, osmium::Location>;
using dense_file_array = osmium::index::map::DenseFileArray<osmium::unsigned_object_id_type, osmium::Location>;

TEST_CASE("Dump SparseMmapArray as array and load it as DenseFileArray") {

    const int fd = osmium::detail::create_tmp_file();
    constexpr const size_t VALUE_SIZE = sizeof(sparse_mmap_array::element_type::second_type);
    constexpr const size_t BUFFER_SIZE = (10L * 1024L * 1024L) / VALUE_SIZE;

    REQUIRE(osmium::file_size(fd) == 0);

    std::vector<osmium::unsigned_object_id_type> ids = {
            1,
            6,
            2 * BUFFER_SIZE,
            2 * BUFFER_SIZE + 1,
            3 * BUFFER_SIZE - 1,
            3 * BUFFER_SIZE,
            3 * BUFFER_SIZE + 1,
            3 * BUFFER_SIZE + 2,
            3 * BUFFER_SIZE + 3,
            3 * BUFFER_SIZE + 4,
            3 * BUFFER_SIZE + 5
    };
    std::vector<osmium::Location> locations = {
            {1.2, 4.5},
            {3.5, -7.2},
            {10.2, 64.5},
            {39.5, -71.2},
            {-1.2, 54.6},
            {-171.2, 9.3},
            {-171.21, 9.26},
            {-171.22, 9.25},
            {-171.23, 9.24},
            {-171.24, 9.23},
            {-171.25, 9.22},
            {-171.26, 9.21},
    };

    constexpr const size_t S = sizeof(dense_file_array::element_type);
    sparse_mmap_array sparse_index;
    for (size_t i = 0; i != ids.size() && i != locations.size(); ++i) {
        sparse_index.set(ids[i], locations[i]);
    }
    sparse_index.sort();
    sparse_index.dump_as_array(fd);

    dense_file_array dense_index{fd};
    REQUIRE(osmium::file_size(fd) >= (*std::max_element(ids.begin(), ids.end()) * S));
    REQUIRE_THROWS_AS(dense_index.get(0), const osmium::not_found&);
    REQUIRE_THROWS_AS(dense_index.get(BUFFER_SIZE - 1), const osmium::not_found&);
    REQUIRE_THROWS_AS(dense_index.get(BUFFER_SIZE), const osmium::not_found&);
    REQUIRE_THROWS_AS(dense_index.get(ids[1] - 1), const osmium::not_found&);
    REQUIRE_THROWS_AS(dense_index.get(ids[1] + 1), const osmium::not_found&);
    REQUIRE_THROWS_AS(dense_index.get(100), const osmium::not_found&);
    REQUIRE_THROWS_AS(dense_index.get(ids[2] - 1), const osmium::not_found&);
    REQUIRE_THROWS_AS(dense_index.get(BUFFER_SIZE), const osmium::not_found&);
    REQUIRE_THROWS_AS(dense_index.get(4 * BUFFER_SIZE), const osmium::not_found&);
    REQUIRE_THROWS_AS(dense_index.get(4 * BUFFER_SIZE - 1), const osmium::not_found&);

    for (size_t i = 0; i != ids.size() && i != locations.size(); ++i) {
        REQUIRE(dense_index.get(ids[i]) == locations[i]);
    }
}
