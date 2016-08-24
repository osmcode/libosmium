/*

  This is a small tool that counts the number of nodes, ways, and relations
  in the input file. It demonstrates the use of file input, handlers, and the
  memory usage utility class.

  The code in this example file is released into the Public Domain.

*/

#include <cstdint>
#include <iostream>

// Allow any format of input files (XML, PBF, ...)
#include <osmium/io/any_input.hpp>

// We want to use the handler interface
#include <osmium/handler.hpp>

// Utility class gives us access to memory usage information
#include <osmium/util/memory.hpp>

// Needed for the osmium::apply() function
#include <osmium/visitor.hpp>

// Handler derive from the osmium::handler::Handler base class. Usually you
// overwrite functions node(), way(), and relation(). Other functions are
// available, too. Read the API documentation for details.
struct CountHandler : public osmium::handler::Handler {

    std::uint64_t nodes     = 0;
    std::uint64_t ways      = 0;
    std::uint64_t relations = 0;

    void node(const osmium::Node&) noexcept {
        ++nodes;
    }

    void way(const osmium::Way&) noexcept {
        ++ways;
    }

    void relation(const osmium::Relation&) noexcept {
        ++relations;
    }

}; // struct CountHandler


int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " OSMFILE\n";
        exit(1);
    }

    osmium::io::File input_file{argv[1]};
    osmium::io::Reader reader{input_file};

    CountHandler handler;
    osmium::apply(reader, handler);
    reader.close();

    std::cout << "Nodes: "     << handler.nodes << "\n";
    std::cout << "Ways: "      << handler.ways << "\n";
    std::cout << "Relations: " << handler.relations << "\n";

    // Because of the huge amount of OSM data, some Osmium-based programs
    // (though not this one) can use huge amounts of data. So checking actual
    // memore usage is often useful and can be done easily with this class.
    // (Currently only works on Linux, not OSX and Windows.)
    osmium::MemoryUsage memory;

    std::cout << "\nMemory used: " << memory.peak() << " MBytes\n";
}

