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
        REQUIRE(!last_block.is_populated());
        REQUIRE(last_block.first_item_type_or_zero == osmium::item_type::undefined);
        REQUIRE(last_block.first_item_id_or_zero == 0);
    }
}

static void require_binary_search_result(
    const osmium::io::PbfBlockIndexTable& table,
    osmium::item_type needle_item_type,
    osmium::object_id_type needle_item_id,
    size_t expected_begin_search,
    size_t expected_end_search,
    size_t expected_result
) {
    size_t begin_search = 0;
    size_t end_search = table.block_starts().size();
    size_t actual_result = table.binary_search_object_guess(needle_item_type, needle_item_id, begin_search, end_search);
    REQUIRE(actual_result == expected_result);
    REQUIRE(begin_search == expected_begin_search);
    REQUIRE(end_search == expected_end_search);
}

/**
 * Can read and index planet.pbf.
 */
static const char* PLANET_FILENAME = "/scratch/osm/planet-231002.osm.pbf";
TEST_CASE("Can read planet quickly") {
    osmium::io::PbfBlockIndexTable table {PLANET_FILENAME};
    REQUIRE(table.block_starts().size() >= 10000);

    SECTION("Can 'randomly' access the first block") {
        auto middle_block = table.get_parsed_block(0, osmium::io::read_meta::no);
        auto it = middle_block.front()->begin<osmium::OSMObject>();
        REQUIRE(it->type() == osmium::item_type::node);
        REQUIRE(it->id() == 1);
    }

    SECTION("Can randomly access some arbitrary block") {
        auto middle_block = table.get_parsed_block(1234, osmium::io::read_meta::no);
        auto it = middle_block.front()->begin<osmium::OSMObject>();
        REQUIRE(it->type() == osmium::item_type::node);
        REQUIRE(it->id() >= 100000000);
    }

    SECTION("Find a late node") {
        auto buffers = table.binary_search_object(osmium::item_type::node, 5301526002, osmium::io::read_meta::no);
        REQUIRE(!buffers.empty());
        bool found = false;
        for (const auto& buffer : buffers) {
            for (auto it = buffer->begin<osmium::OSMObject>(); it != buffer->end<osmium::OSMObject>(); ++it) {
                if (it->type() == osmium::item_type::node && it->id() == 5301526002) {
                    found = true;
                    break;
                }
            }
            if (found)
                break;
        }
        REQUIRE(found);
    }

    SECTION("Find an early node") {
        auto buffers = table.binary_search_object(osmium::item_type::node, 123, osmium::io::read_meta::no);
        REQUIRE(!buffers.empty());
        bool found = false;
        for (const auto& buffer : buffers) {
            for (auto it = buffer->begin<osmium::OSMObject>(); it != buffer->end<osmium::OSMObject>(); ++it) {
                if (it->type() == osmium::item_type::node && it->id() == 123) {
                    found = true;
                    break;
                }
            }
            if (found)
                break;
        }
        REQUIRE(found);
    }
}

/**
 * Can read and index some reasonable osm.pbf files.
 */
TEST_CASE("Access only middle block of PBF files, and check binary search") {
    osmium::io::PbfBlockIndexTable table {with_data_dir("t/io/data-n5w1r3.osm.pbf")};
    REQUIRE(table.block_starts().size() == 3);

    /* Test without any loaded blocks: */
    REQUIRE(table.block_starts()[0].first_item_id_or_zero == 0);
    REQUIRE(table.block_starts()[0].first_item_type_or_zero == osmium::item_type::undefined);
    REQUIRE(table.block_starts()[1].first_item_id_or_zero == 0);
    REQUIRE(table.block_starts()[1].first_item_type_or_zero == osmium::item_type::undefined);
    REQUIRE(table.block_starts()[2].first_item_id_or_zero == 0);
    REQUIRE(table.block_starts()[2].first_item_type_or_zero == osmium::item_type::undefined);
    require_binary_search_result(table, osmium::item_type::node, 5, 0, 3, 1);
    require_binary_search_result(table, osmium::item_type::node, 10, 0, 3, 1);
    require_binary_search_result(table, osmium::item_type::node, 12, 0, 3, 1);
    require_binary_search_result(table, osmium::item_type::node, 25, 0, 3, 1);
    require_binary_search_result(table, osmium::item_type::node, 40, 0, 3, 1);
    require_binary_search_result(table, osmium::item_type::way, 4, 0, 3, 1);
    require_binary_search_result(table, osmium::item_type::way, 20, 0, 3, 1);
    require_binary_search_result(table, osmium::item_type::way, 40, 0, 3, 1);
    require_binary_search_result(table, osmium::item_type::relation, 5, 0, 3, 1);
    require_binary_search_result(table, osmium::item_type::relation, 20, 0, 3, 1);
    require_binary_search_result(table, osmium::item_type::relation, 30, 0, 3, 1);
    require_binary_search_result(table, osmium::item_type::relation, 35, 0, 3, 1);

    /* Test with only the middle block loaded: */
    auto middle_block = table.get_parsed_block(1, osmium::io::read_meta::yes);
    REQUIRE(table.block_starts()[0].first_item_id_or_zero == 0);
    REQUIRE(table.block_starts()[0].first_item_type_or_zero == osmium::item_type::undefined);
    REQUIRE(table.block_starts()[1].first_item_id_or_zero == 20);
    REQUIRE(table.block_starts()[1].first_item_type_or_zero == osmium::item_type::way);
    REQUIRE(table.block_starts()[1].is_populated());
    REQUIRE(table.block_starts()[2].first_item_id_or_zero == 0);
    REQUIRE(table.block_starts()[2].first_item_type_or_zero == osmium::item_type::undefined);
    REQUIRE(!table.block_starts()[2].is_populated());
    {
        REQUIRE(middle_block.size() == 1);
        auto it = middle_block[0]->begin<osmium::OSMObject>();
        REQUIRE(it->id() == 20);
        REQUIRE(it->type() == osmium::item_type::way);
        ++it;
        REQUIRE(it == middle_block[0]->end<osmium::OSMObject>());
    }
    require_binary_search_result(table, osmium::item_type::node, 5, 0, 1, 0);
    require_binary_search_result(table, osmium::item_type::node, 10, 0, 1, 0);
    require_binary_search_result(table, osmium::item_type::node, 12, 0, 1, 0);
    require_binary_search_result(table, osmium::item_type::node, 25, 0, 1, 0);
    require_binary_search_result(table, osmium::item_type::node, 40, 0, 1, 0);
    require_binary_search_result(table, osmium::item_type::way, 4, 0, 1, 0);
    require_binary_search_result(table, osmium::item_type::way, 20, 1, 3, 2);
    require_binary_search_result(table, osmium::item_type::way, 40, 1, 3, 2);
    require_binary_search_result(table, osmium::item_type::relation, 5, 1, 3, 2);
    require_binary_search_result(table, osmium::item_type::relation, 20, 1, 3, 2);
    require_binary_search_result(table, osmium::item_type::relation, 30, 1, 3, 2);
    require_binary_search_result(table, osmium::item_type::relation, 35, 1, 3, 2);

    /* Test with only the first two blocks loaded: */
    auto first_block = table.get_parsed_block(0, osmium::io::read_meta::no);
    REQUIRE(table.block_starts()[0].first_item_id_or_zero == 10);
    REQUIRE(table.block_starts()[0].first_item_type_or_zero == osmium::item_type::node);
    REQUIRE(table.block_starts()[1].first_item_id_or_zero == 20);
    REQUIRE(table.block_starts()[1].first_item_type_or_zero == osmium::item_type::way);
    REQUIRE(table.block_starts()[2].first_item_id_or_zero == 0);
    REQUIRE(table.block_starts()[2].first_item_type_or_zero == osmium::item_type::undefined);
    {
        REQUIRE(first_block.size() == 1);
        auto it = first_block[0]->begin<osmium::OSMObject>();
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
        REQUIRE(it == first_block[0]->end<osmium::OSMObject>());
    }
    require_binary_search_result(table, osmium::item_type::node, 5, 0, 0, 3);
    require_binary_search_result(table, osmium::item_type::node, 10, 0, 1, 0);
    require_binary_search_result(table, osmium::item_type::node, 12, 0, 1, 0);
    require_binary_search_result(table, osmium::item_type::node, 25, 0, 1, 0);
    require_binary_search_result(table, osmium::item_type::node, 40, 0, 1, 0);
    require_binary_search_result(table, osmium::item_type::way, 4, 0, 1, 0);
    require_binary_search_result(table, osmium::item_type::way, 20, 1, 3, 2);
    require_binary_search_result(table, osmium::item_type::way, 40, 1, 3, 2);
    require_binary_search_result(table, osmium::item_type::relation, 5, 1, 3, 2);
    require_binary_search_result(table, osmium::item_type::relation, 20, 1, 3, 2);
    require_binary_search_result(table, osmium::item_type::relation, 30, 1, 3, 2);
    require_binary_search_result(table, osmium::item_type::relation, 35, 1, 3, 2);

    /* Test with all blocks loaded: */
    auto last_block = table.get_parsed_block(2, osmium::io::read_meta::no);
    REQUIRE(table.block_starts()[0].first_item_id_or_zero == 10);
    REQUIRE(table.block_starts()[0].first_item_type_or_zero == osmium::item_type::node);
    REQUIRE(table.block_starts()[1].first_item_id_or_zero == 20);
    REQUIRE(table.block_starts()[1].first_item_type_or_zero == osmium::item_type::way);
    REQUIRE(table.block_starts()[2].first_item_id_or_zero == 30);
    REQUIRE(table.block_starts()[2].first_item_type_or_zero == osmium::item_type::relation);
    {
        REQUIRE(last_block.size() == 1);
        auto it = last_block[0]->begin<osmium::OSMObject>();
        REQUIRE(it->id() == 30);
        REQUIRE(it->type() == osmium::item_type::relation);
        ++it;
        REQUIRE(it->id() == 31);
        REQUIRE(it->type() == osmium::item_type::relation);
        ++it;
        REQUIRE(it->id() == 32);
        REQUIRE(it->type() == osmium::item_type::relation);
        ++it;
        REQUIRE(it == last_block[0]->end<osmium::OSMObject>());
    }
    require_binary_search_result(table, osmium::item_type::node, 5, 0, 0, 3);
    require_binary_search_result(table, osmium::item_type::node, 10, 0, 1, 0);
    require_binary_search_result(table, osmium::item_type::node, 12, 0, 1, 0);
    require_binary_search_result(table, osmium::item_type::node, 25, 0, 1, 0);
    require_binary_search_result(table, osmium::item_type::node, 40, 0, 1, 0);
    require_binary_search_result(table, osmium::item_type::way, 4, 0, 1, 0);
    require_binary_search_result(table, osmium::item_type::way, 20, 1, 2, 1);
    require_binary_search_result(table, osmium::item_type::way, 40, 1, 2, 1);
    require_binary_search_result(table, osmium::item_type::relation, 5, 1, 2, 1);
    require_binary_search_result(table, osmium::item_type::relation, 20, 1, 2, 1);
    require_binary_search_result(table, osmium::item_type::relation, 30, 2, 3, 2);
    require_binary_search_result(table, osmium::item_type::relation, 35, 2, 3, 2);
}

/**
 * Sanity-check the sizes.
 */
TEST_CASE("check size of pbf_block_start") {
    if (sizeof(size_t) == 4) {
        REQUIRE(sizeof(osmium::io::pbf_block_start) == 16);
    } else if (sizeof(size_t) == 8) {
        REQUIRE(sizeof(osmium::io::pbf_block_start) == 24);
    } else {
        // Print a warning?
        REQUIRE(sizeof(osmium::io::pbf_block_start) <= 24);
    }
}

/**
 * This test is mostly just paranoia.
 */
TEST_CASE("binsearch_middle actually returns the middle") {
    /* Length 2 */
    REQUIRE(osmium::io::detail::binsearch_middle(0, 2) == 1);
    REQUIRE(osmium::io::detail::binsearch_middle(1, 3) == 2);
    REQUIRE(osmium::io::detail::binsearch_middle(2, 4) == 3);
    REQUIRE(osmium::io::detail::binsearch_middle(3, 5) == 4);
    /* Length 3 */
    REQUIRE(osmium::io::detail::binsearch_middle(0, 3) == 1);
    REQUIRE(osmium::io::detail::binsearch_middle(1, 4) == 2);
    REQUIRE(osmium::io::detail::binsearch_middle(2, 5) == 3);
    /* Length 4 */
    REQUIRE(osmium::io::detail::binsearch_middle(0, 4) == 2);
    REQUIRE(osmium::io::detail::binsearch_middle(1, 5) == 3);
    REQUIRE(osmium::io::detail::binsearch_middle(2, 6) == 4);
    /* Length 5 */
    REQUIRE(osmium::io::detail::binsearch_middle(0, 5) == 2);
    REQUIRE(osmium::io::detail::binsearch_middle(1, 6) == 3);
    REQUIRE(osmium::io::detail::binsearch_middle(2, 7) == 4);
    /* Length 1000 */
    REQUIRE(osmium::io::detail::binsearch_middle(0, 1000) == 500);
    REQUIRE(osmium::io::detail::binsearch_middle(1, 1001) == 501);
    REQUIRE(osmium::io::detail::binsearch_middle(2, 1002) == 502);
    /* Length 2**30 */
    REQUIRE(osmium::io::detail::binsearch_middle(0, 1073741824) == 536870912);
    REQUIRE(osmium::io::detail::binsearch_middle(1, 1073741825) == 536870913);
    REQUIRE(osmium::io::detail::binsearch_middle(2, 1073741826) == 536870914);
    /* Length 2**31 */
    REQUIRE(osmium::io::detail::binsearch_middle(0, 2147483648) == 1073741824);
    REQUIRE(osmium::io::detail::binsearch_middle(1, 2147483649) == 1073741825);
    REQUIRE(osmium::io::detail::binsearch_middle(2, 2147483650) == 1073741826);
    /* Length 2**31 + 1 */
    REQUIRE(osmium::io::detail::binsearch_middle(0, 2147483649) == 1073741824);
    REQUIRE(osmium::io::detail::binsearch_middle(1, 2147483650) == 1073741825);
    REQUIRE(osmium::io::detail::binsearch_middle(2, 2147483651) == 1073741826);
    /* Length 2**31 + 2 */
    REQUIRE(osmium::io::detail::binsearch_middle(0, 2147483650) == 1073741825);
    REQUIRE(osmium::io::detail::binsearch_middle(1, 2147483651) == 1073741826);
    REQUIRE(osmium::io::detail::binsearch_middle(2, 2147483652) == 1073741827);
    /* Length 2**32 - 3 */
    REQUIRE(osmium::io::detail::binsearch_middle(0, 4294967293) == 2147483646);
    REQUIRE(osmium::io::detail::binsearch_middle(1, 4294967294) == 2147483647);
    REQUIRE(osmium::io::detail::binsearch_middle(2, 4294967295) == 2147483648);
    /* Length 2**32 - 2 */
    REQUIRE(osmium::io::detail::binsearch_middle(0, 4294967294) == 2147483647);
    REQUIRE(osmium::io::detail::binsearch_middle(1, 4294967295) == 2147483648);
    /* Length 2**32 - 1 */
    REQUIRE(osmium::io::detail::binsearch_middle(0, 4294967295) == 2147483647);
    /* Length 2**64 - 2, if this is a 64-bit system, else 2**32 - 2 */
    REQUIRE(osmium::io::detail::binsearch_middle(1, SIZE_MAX) == SIZE_MAX / 2 + 1);
    /* Length 2**64 - 1, if this is a 64-bit system, else 2**32 - 1 */
    REQUIRE(osmium::io::detail::binsearch_middle(0, SIZE_MAX) == SIZE_MAX / 2);
}
