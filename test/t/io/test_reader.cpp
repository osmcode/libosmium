#include "catch.hpp"

#include <osmium/handler.hpp>
#include <osmium/io/any_input.hpp>
#include <osmium/visitor.hpp>
#include <osmium/memory/buffer.hpp>

TEST_CASE("Reader") {

    SECTION("reader can be initialized with file") {
        osmium::io::File file("t/io/data.osm"); 
        osmium::io::Reader reader(file);
        osmium::handler::Handler handler;

        osmium::apply(reader, handler);
    }

    SECTION("reader can be initialized with string") {
        osmium::io::Reader reader("t/io/data.osm");
        osmium::handler::Handler handler;

        osmium::apply(reader, handler);
    }

    SECTION("should return invalid buffer after eof") {
        osmium::io::File file("t/io/data.osm"); 
        osmium::io::Reader reader(file);

        while (osmium::memory::Buffer buffer = reader.read()) {
        }

        // extra read always returns invalid buffer
        osmium::memory::Buffer buffer = reader.read();
        REQUIRE(!buffer);
    }

    SECTION("should not hang when apply() is called twice on reader") {
        osmium::io::File file("t/io/data.osm"); 
        osmium::io::Reader reader(file);
        osmium::handler::Handler handler;

        osmium::apply(reader, handler);
        osmium::apply(reader, handler);
    }

}

