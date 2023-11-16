#include "catch.hpp"

#include "utils.hpp"

#include <osmium/io/pbf_input_randomaccess.hpp>

/**
 * Can read and index some reasonable osm.pbf files.
 */
TEST_CASE("Read normal PBF files") {
    struct file_expectation {
        const char* filename;
        size_t num_blocks;
        size_t last_block_offset;
        size_t last_block_datasize;
    };
    const file_expectation file_expectations[] = {
        { "t/io/data_pbf_version-1-densenodes.osm.pbf", 1, 73, 63 },
        { "t/io/data_pbf_version-1.osm.pbf", 1, 59, 60 },
        { "t/io/deleted_nodes.osh.pbf", 1, 110, 79 },
        { "t/io/data-n5w1r3.osm.pbf", 3, 270, 146 }
    };
    for (auto const& expect : file_expectations) {
        const osmium::io::PbfBlockIndexTable table {with_data_dir(expect.filename)};
        const auto& table_blocklist = table.block_starts();
        REQUIRE(table_blocklist.size() == expect.num_blocks);
        const auto& last_block = table_blocklist[table_blocklist.size() - 1];
        REQUIRE(last_block.file_offset == expect.last_block_offset);
        REQUIRE(last_block.datasize == expect.last_block_datasize);
        // Since no block has been decoded yet, this must still be zero:
        REQUIRE(last_block.first_item_type_or_zero == osmium::item_type::undefined);
        REQUIRE(last_block.first_item_id_or_zero == 0);
    }
}
