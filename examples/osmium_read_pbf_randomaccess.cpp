/*

  EXAMPLE osmium_read_pbf

  Reads and discards the contents of the input file.
  (It can be used for timing.)

  DEMONSTRATES USE OF:
  * file input

  LICENSE
  The code in this example file is released into the Public Domain.

*/

//#include "utils.hpp"

#include <iostream> // std::cout, std::cerr

#include <osmium/io/pbf_input_randomaccess.hpp>
#include <osmium/visitor.hpp>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " OSMFILE\n";
        std::cerr << "(Will try to read some random object from it.)\n";
        return 1;
    }

    osmium::io::PbfBlockIndexTable table {argv[1]};
    std::cout << "File has " << table.block_starts().size() << " blocks." << std::endl;
    auto buffer = table.binary_search_object(osmium::item_type::way, 40106791, osmium::io::read_meta::no);
    std::cout << "buffer1 has capacity=" << buffer.capacity() << ", committed=" << buffer.committed() << ", written=" << buffer.written() << std::endl;
    if (!buffer) {
        std::cout << "File definitely doesn't contain way 40106791." << std::endl;
    } else {
        bool found = false;
        size_t num_seen = 0;
        for (auto it = buffer.begin<osmium::OSMObject>(); it != buffer.end<osmium::OSMObject>(); ++it) {
            num_seen += 1;
            if (it->type() == osmium::item_type::way && it->id() == 40106791) {
                found = true;
                std::cout << "File contains way 40106791!\n";
                auto& way = static_cast<osmium::Way&>(*it);
                std::cout << "Nodes are: ";
                auto& nodes = way.nodes();
                bool is_first = true;
                for (auto& ref : nodes) {
                    if (is_first) {
                        is_first = false;
                    } else {
                        std::cout << ", ";
                    }
                    std::cout << ref.ref();
                }
                std::cout << "." << std::endl;
                break;
            }
        }
        if (found) {
        } else {
            std::cerr << "File contains a promising-looking block, but it does not contain way 40106791.\n";
        }
    }
    auto buffer2 = table.binary_search_object(osmium::item_type::relation, 123456, osmium::io::read_meta::no);
    std::cout << "buffer2 has capacity=" << buffer2.capacity() << ", committed=" << buffer2.committed() << ", written=" << buffer2.written() << std::endl;
}

