#include "catch.hpp"

#include "utils.hpp"

#include <osmium/io/pbf_input_randomaccess.hpp>
#include <osmium/visitor.hpp>

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

/**
 * Can read and index some reasonable osm.pbf files.
 */
TEST_CASE("Access only middle block of PBF files") {
    osmium::io::PbfBlockIndexTable table {with_data_dir("t/io/data-n5w1r3.osm.pbf")};
    REQUIRE(table.block_starts().size() == 3);
    REQUIRE(table.block_starts()[0].first_item_id_or_zero == 0);
    REQUIRE(table.block_starts()[0].first_item_type_or_zero == osmium::item_type::undefined);
    REQUIRE(table.block_starts()[1].first_item_id_or_zero == 0);
    REQUIRE(table.block_starts()[1].first_item_type_or_zero == osmium::item_type::undefined);
    REQUIRE(table.block_starts()[2].first_item_id_or_zero == 0);
    REQUIRE(table.block_starts()[2].first_item_type_or_zero == osmium::item_type::undefined);
    auto middle_block = table.get_parsed_block(1, osmium::io::read_meta::yes);
    REQUIRE(table.block_starts()[0].first_item_id_or_zero == 0);
    REQUIRE(table.block_starts()[0].first_item_type_or_zero == osmium::item_type::undefined);
    REQUIRE(table.block_starts()[1].first_item_id_or_zero == 20);
    REQUIRE(table.block_starts()[1].first_item_type_or_zero == osmium::item_type::way);
    REQUIRE(table.block_starts()[2].first_item_id_or_zero == 0);
    REQUIRE(table.block_starts()[2].first_item_type_or_zero == osmium::item_type::undefined);
    {
        auto it = middle_block.begin<osmium::OSMObject>();
        REQUIRE(it->id() == 20);
        REQUIRE(it->type() == osmium::item_type::way);
        ++it;
        REQUIRE(it == middle_block.end<osmium::OSMObject>());
    }
    auto first_block = table.get_parsed_block(0, osmium::io::read_meta::no);
    REQUIRE(table.block_starts()[0].first_item_id_or_zero == 10);
    REQUIRE(table.block_starts()[0].first_item_type_or_zero == osmium::item_type::node);
    REQUIRE(table.block_starts()[1].first_item_id_or_zero == 20);
    REQUIRE(table.block_starts()[1].first_item_type_or_zero == osmium::item_type::way);
    REQUIRE(table.block_starts()[2].first_item_id_or_zero == 0);
    REQUIRE(table.block_starts()[2].first_item_type_or_zero == osmium::item_type::undefined);
    {
        auto it = first_block.begin<osmium::OSMObject>();
        REQUIRE(it->id() == 10);
        REQUIRE(it->type() == osmium::item_type::node);
        ++it;
        REQUIRE(it->id() == 11);
        REQUIRE(it->type() == osmium::item_type::node);
        ++it;
        REQUIRE(it->id() == 12);
        REQUIRE(it->type() == osmium::item_type::node);
        ++it;
        REQUIRE(it->id() == 13);
        REQUIRE(it->type() == osmium::item_type::node);
        ++it;
        REQUIRE(it->id() == 14);
        REQUIRE(it->type() == osmium::item_type::node);
        ++it;
        REQUIRE(it == first_block.end<osmium::OSMObject>());
    }
}
