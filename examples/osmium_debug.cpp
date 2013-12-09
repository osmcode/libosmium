/*

  This is a small tool to dump the contents of the input file.

  The code in this example file is released into the Public Domain.

*/

#include <iostream>

#include <osmium/io/any_input.hpp>
#include <osmium/osm/dump.hpp>

int main(int argc, char* argv[]) {
    std::ios_base::sync_with_stdio(false);

    if (argc < 2 || argc > 3) {
        std::cerr << "Usage: " << argv[0] << " OSMFILE [TYPES]\n";
        std::cerr << "TYPES can be any combination of 'n', 'w', 'r', and 'c' to indicate what types of OSM entities you want (default: all).\n";
        exit(1);
    }

    osmium::osm_entity::flags read_types = osmium::osm_entity::flags::all;

    if (argc == 3) {
        read_types = osmium::osm_entity::flags::nothing;
        std::string types = argv[2];
        if (types.find('n') != std::string::npos) read_types |= osmium::osm_entity::flags::node;
        if (types.find('w') != std::string::npos) read_types |= osmium::osm_entity::flags::way;
        if (types.find('r') != std::string::npos) read_types |= osmium::osm_entity::flags::relation;
        if (types.find('c') != std::string::npos) read_types |= osmium::osm_entity::flags::changeset;
    }

    osmium::io::Reader reader(argv[1], read_types);
    osmium::io::Header header = reader.header();

    std::cout << "HEADER:\n  generator=" << header.get("generator") << "\n";

    for (auto& bbox : header.boxes()) {
        std::cout << "  bbox=" << bbox << "\n";
    }

    osmium::osm::Dump dump(std::cout);
    while (osmium::memory::Buffer buffer = reader.read()) {
        osmium::apply(buffer, dump);
    }

    google::protobuf::ShutdownProtobufLibrary();
}

