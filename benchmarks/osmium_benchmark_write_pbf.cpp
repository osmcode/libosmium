/*

  The code in this file is released into the Public Domain.

*/

#include <osmium/io/any_input.hpp>
#include <osmium/io/any_output.hpp>

#include <cstdlib>
#include <exception>
#include <iostream>
#include <string>
#include <utility>

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " INPUT-FILE OUTPUT-FILE\n";
        return 1;
    }

    try {
        const std::string input_filename{argv[1]};
        const std::string output_filename{argv[2]};

        osmium::io::Reader reader{input_filename};
        const osmium::io::File output_file{output_filename, "pbf"};
        const osmium::io::Header header;
        osmium::io::Writer writer{output_file, header, osmium::io::overwrite::allow};

        while (osmium::memory::Buffer buffer = reader.read()) { // NOLINT(bugprone-use-after-move) Bug in clang-tidy https://bugs.llvm.org/show_bug.cgi?id=36516
            writer(std::move(buffer));
        }

        writer.close();
        reader.close();
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }

    return 0;
}

