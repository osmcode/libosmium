/*

  This is a small tool to dump the contents of the input file.

  The code in this example file is released into the Public Domain.

*/

#include <iostream>

#include <osmium/io/any_input.hpp>
#include <osmium/osm/dump.hpp>

int main(int argc, char* argv[]) {
//    std::ios_base::sync_with_stdio(false);

    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " OSMFILE\n";
        exit(1);
    }

    osmium::io::Reader reader(argv[1]);
    osmium::io::Meta meta = reader.open();

    std::cout << "META:\n  generator=" << meta.generator() << "\n";
    std::cout << "  bounds=" << meta.bounds() << "\n";

    osmium::osm::Dump dump(std::cout);
    while (osmium::memory::Buffer buffer = reader.read()) {
        osmium::osm::apply_visitor(dump, buffer);
    }

    google::protobuf::ShutdownProtobufLibrary();
}

