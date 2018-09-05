#include "catch.hpp"

#include <osmium/index/detail/tmpfile.hpp>
#include <osmium/index/map/dense_file_array.hpp>
#include <osmium/index/map/sparse_mmap_array.hpp>
#include <osmium/index/node_locations_map.hpp>
#include <osmium/osm/node_ref.hpp>
#include <osmium/osm/types.hpp>
#include <osmium/util/file.hpp>

#include <vector>

using sparse_mmap_array = osmium::index::map::SparseMmapArray<osmium::unsigned_object_id_type, osmium::Location>;
using dense_file_array = osmium::index::map::DenseFileArray<osmium::unsigned_object_id_type, osmium::Location>;

TEST_CASE("Dump SparseMmapArray as array and load it as DenseFileArray") {

    const int fd = osmium::detail::create_tmp_file();
    constexpr const size_t value_size = sizeof(sparse_mmap_array::element_type::second_type);
    constexpr const size_t buffer_size = (10L * 1024L * 1024L) / value_size;

    REQUIRE(osmium::file_size(fd) == 0);

    std::vector<osmium::NodeRef> refs = {
        osmium::NodeRef{1,                   osmium::Location{1.2, 4.5}},
        osmium::NodeRef{6,                   osmium::Location{3.5, -7.2}},
        osmium::NodeRef{2 * buffer_size,     osmium::Location{10.2, 64.5}},
        osmium::NodeRef{2 * buffer_size + 1, osmium::Location{39.5, -71.2}},
        osmium::NodeRef{3 * buffer_size - 1, osmium::Location{-1.2, 54.6}},
        osmium::NodeRef{3 * buffer_size,     osmium::Location{-171.2, 9.3}},
        osmium::NodeRef{3 * buffer_size + 1, osmium::Location{-171.21, 9.26}},
        osmium::NodeRef{3 * buffer_size + 2, osmium::Location{-171.22, 9.25}},
        osmium::NodeRef{3 * buffer_size + 3, osmium::Location{-171.24, 9.23}},
        osmium::NodeRef{3 * buffer_size + 4, osmium::Location{-171.25, 9.22}},
        osmium::NodeRef{3 * buffer_size + 5, osmium::Location{-171.26, 9.21}}
    };

    sparse_mmap_array sparse_index;
    for (auto& r : refs) {
        sparse_index.set(r.ref(), r.location());
    }
    sparse_index.sort();
    sparse_index.dump_as_array(fd);

    dense_file_array dense_index{fd};
    const auto max_id_in_refs = std::max_element(refs.begin(), refs.end())->ref();
    REQUIRE(osmium::file_size(fd) >= max_id_in_refs * sizeof(osmium::Location));
    REQUIRE_THROWS_AS(dense_index.get(0),                   const osmium::not_found&);
    REQUIRE_THROWS_AS(dense_index.get(buffer_size - 1),     const osmium::not_found&);
    REQUIRE_THROWS_AS(dense_index.get(buffer_size),         const osmium::not_found&);
    REQUIRE_THROWS_AS(dense_index.get(refs[1].ref() - 1),   const osmium::not_found&);
    REQUIRE_THROWS_AS(dense_index.get(refs[1].ref() + 1),   const osmium::not_found&);
    REQUIRE_THROWS_AS(dense_index.get(100),                 const osmium::not_found&);
    REQUIRE_THROWS_AS(dense_index.get(refs[2].ref() - 1),   const osmium::not_found&);
    REQUIRE_THROWS_AS(dense_index.get(buffer_size),         const osmium::not_found&);
    REQUIRE_THROWS_AS(dense_index.get(4 * buffer_size),     const osmium::not_found&);
    REQUIRE_THROWS_AS(dense_index.get(4 * buffer_size - 1), const osmium::not_found&);

    for (auto& r : refs) {
        REQUIRE(dense_index.get(r.ref()) == r.location());
    }
}
