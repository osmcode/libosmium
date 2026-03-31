#include "catch.hpp"

#include "utils.hpp"

#include <osmium/io/detail/pbf_input_format.hpp>
#include <osmium/io/pbf_input.hpp>
#include <osmium/io/reader.hpp>
#include <osmium/osm/object.hpp>

#include <array>

TEST_CASE("Get supported PBF compression types") {
    const auto types = osmium::io::supported_pbf_compression_types();
    REQUIRE(types.size() >= 2);
    REQUIRE(types[0] == "none");
    REQUIRE(types[1] == "zlib");
}

/**
 * Osmosis writes PBF with changeset=-1 if its input file did not contain the changeset field.
 * The default value of the version field is -1 in the OSM.PBF format.
 *
 * t/io/data_pbf_version-1.osm.pbf was created using the following steps:
 *
 * * Convert t/io/data_pbf_version-1.osm to PBF format using Osmium Tool:
 *   `osmium cat -o tmp.osm.pbf --output-format pbf,add_metadata=no /data_pbf_version-1.osm.pbf`)
 * * Convert that file using Osmosis:
 *   `osmosis --read-pbf file=tmp.osm.pbf --write-pbf file=data_pbf_version-1.osm.pbf compress=none usedense=false`
 */
TEST_CASE("Read PBF file with version=-1 and changeset=-1 (written by Osmosis)") {
    const osmium::memory::Buffer buffer = osmium::io::read_file(with_data_dir("t/io/data_pbf_version-1.osm.pbf"));
    // one node should be in this buffer
    const osmium::OSMObject& object = *(buffer.cbegin<osmium::OSMObject>());
    REQUIRE(object.version() == 0);
    REQUIRE(object.changeset() == 0);
}

/**
 * Osmosis writes PBF with changeset=-1 if its input file did not contain the changeset field.
 * The default value of the version field is -1 in the OSM.PBF format.
 *
 * t/io/data_pbf_version-1.osm.pbf was created using the following steps:
 *
 * * Convert t/io/data_pbf_version-1.osm to PBF format using Osmium Tool:
 *   `osmium cat -o tmp.osm.pbf --output-format pbf,add_metadata=no /data_pbf_version-1.osm.pbf`)
 * * Convert that file using Osmosis:
 *   `osmosis --read-pbf file=tmp.osm.pbf --write-pbf file=data_pbf_version-1-densenodes.osm.pbf compress=none usedense=true`
 */
TEST_CASE("Read PBF file with version=-1 and changeset=-1 and DenseNodes (written by Osmosis)") {
    const osmium::memory::Buffer buffer = osmium::io::read_file(with_data_dir("t/io/data_pbf_version-1-densenodes.osm.pbf"));
    // one node should be in this buffer
    const osmium::OSMObject& object = *(buffer.cbegin<osmium::OSMObject>());
    REQUIRE(object.version() == 0);
    REQUIRE(object.changeset() == 0);
}

TEST_CASE("get size in network byte order") {
    const std::array<char, 4> data1 = { 0, 0, 0, 1 };
    REQUIRE(osmium::io::detail::PBFParser::get_size_in_network_byte_order(data1.data()) == 1);

    const std::array<char, 4> data127 = { 0, 0, 0, 127 };
    REQUIRE(osmium::io::detail::PBFParser::get_size_in_network_byte_order(data127.data()) == 127);

    const std::array<char, 4> data128 = { 0, 0, 0, static_cast<char>(128) };
    REQUIRE(osmium::io::detail::PBFParser::get_size_in_network_byte_order(data128.data()) == 128);

    const std::array<char, 4> data65535 = { 0, 0, static_cast<char>(255), static_cast<char>(255) };
    REQUIRE(osmium::io::detail::PBFParser::get_size_in_network_byte_order(data65535.data()) == 65535);
}
