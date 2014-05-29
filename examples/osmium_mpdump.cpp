/*

  This is an example tool that creates multipolygons from OSM data
  and dumps them to stdout.

  The code in this example file is released into the Public Domain.

*/

#include <iostream>

#include <osmium/io/any_input.hpp>

#include <osmium/index/map/dummy.hpp>
#include <osmium/index/map/sparse_table.hpp>
#include <osmium/handler/node_locations_for_ways.hpp>
#include <osmium/area/collector.hpp>
#include <osmium/area/assembler.hpp>
#include <osmium/visitor.hpp>
#include <osmium/osm/dump.hpp>
#include <osmium/geom/wkt.hpp>

typedef osmium::index::map::Dummy<osmium::unsigned_object_id_type, osmium::Location> index_neg_type;
typedef osmium::index::map::SparseTable<osmium::unsigned_object_id_type, osmium::Location> index_pos_type;
typedef osmium::handler::NodeLocationsForWays<index_pos_type, index_neg_type> location_handler_type;

class WKTDump : public osmium::handler::Handler {

    osmium::geom::WKTFactory m_factory{};

public:

    void area(const osmium::Area& area) {
        std::cout << m_factory.create_multipolygon(area) << "\n";
    }

}; // class WKTDump

int main(int argc, char* argv[]) {

    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " OSMFILE\n";
        exit(1);
    }

    osmium::io::File infile(argv[1]);

    osmium::area::Assembler::config_type assembler_config;
    osmium::area::Collector<osmium::area::Assembler> collector(assembler_config);

    std::cout << "Pass 1...\n";
    osmium::io::Reader reader1(infile, osmium::osm_entity::flags::relation);
    collector.read_relations(reader1);
    std::cout << "Pass 1 done\n";

    std::cout << "Memory:\n";
    collector.used_memory();

    index_pos_type index_pos;
    index_neg_type index_neg;
    location_handler_type location_handler(index_pos, index_neg);
    location_handler.ignore_errors(); // XXX

    osmium::osm::Dump dump(std::cout);
    WKTDump wktdump;

    std::cout << "Pass 2...\n";
    osmium::io::Reader reader2(infile);
    osmium::apply(reader2, location_handler, collector.handler([&dump](const osmium::memory::Buffer& buffer) {
        osmium::apply(buffer, dump);
    }));
    std::cout << "Pass 2 done\n";

    std::cout << "Memory:\n";
    collector.used_memory();

    google::protobuf::ShutdownProtobufLibrary();
}

