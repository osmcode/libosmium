/*

  This is a small tool that reads and discards the contents of the input file.
  (Used for timing.)

  The code in this example file is released into the Public Domain.

*/

#include <iostream>

#include <osmium/io/any_input.hpp>

int main(int argc, char* argv[]) {

    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " OSMFILE THREADS\n";
        exit(1);
    }

    osmium::OSMFile infile(argv[1]);
    osmium::io::Reader reader(infile);
    osmium::io::Meta meta = reader.open();

    while (osmium::memory::Buffer buffer = reader.read()) {
        // do nothing
        delete[] buffer.data();
    }

    google::protobuf::ShutdownProtobufLibrary();
}

