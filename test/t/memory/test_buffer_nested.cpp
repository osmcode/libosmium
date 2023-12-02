#include "catch.hpp"

#include <osmium/memory/buffer.hpp>
#include <osmium/builder/osm_object_builder.hpp>

#include <array>
#include <stdexcept>

static void commit_dummy_data(osmium::memory::Buffer& buffer, osmium::object_id_type id) {
    {
        osmium::builder::NodeBuilder node_builder{buffer};
        node_builder.set_id(id);
        node_builder.set_user("some_silly_long_username_that_eats_up_some_memory_more_quickly_than_a_bunch_of_numbers_would");
    }
    buffer.commit();
}

static osmium::memory::Buffer make_populated_buffer(size_t nodes, osmium::memory::Buffer::auto_grow auto_grow) {
    osmium::memory::Buffer buffer{512, auto_grow};
    for (size_t i = 0; i < nodes; ++i) {
        commit_dummy_data(buffer, i);
    }
    return buffer;
}

TEST_CASE("Can populate and iterate auto_grow::yes") {
    osmium::memory::Buffer buffer = make_populated_buffer(10, osmium::memory::Buffer::auto_grow::yes);
    REQUIRE(buffer);
    REQUIRE(buffer.is_aligned());
    REQUIRE(buffer.written() >= 1360);
    REQUIRE(buffer.written() <= 1440);
    REQUIRE(buffer.committed() == buffer.written());
    REQUIRE(buffer.capacity() > buffer.written());
    REQUIRE(!buffer.has_nested_buffers());

    size_t items = 0;
    for (auto it = buffer.begin(); it != buffer.end(); ++it) {
        items += 1;
    }
    REQUIRE(items == 10);

    size_t nodes = 0;
    for (auto it = buffer.begin<osmium::Node>(); it != buffer.end<osmium::Node>(); ++it) {
        nodes += 1;
    }
    REQUIRE(nodes == 10);

    // REQUIRE that none of these functions raise anything:
    buffer.begin();
    buffer.begin<osmium::Node>();
    buffer.end();
    buffer.end<osmium::Node>();
    buffer.cbegin();
    buffer.cbegin<osmium::Node>();
    buffer.cend();
    buffer.cend<osmium::Node>();
    buffer.get_iterator(0);
    buffer.get_iterator<osmium::Node>(0);
    buffer.select<osmium::Node>();
}

TEST_CASE("Can populate but NOT iterate nested auto_grow::internal") {
    osmium::memory::Buffer buffer = make_populated_buffer(10, osmium::memory::Buffer::auto_grow::internal);
    REQUIRE(buffer);
    REQUIRE(buffer.is_aligned());
    REQUIRE(buffer.capacity() == 512);
    REQUIRE(buffer.written() > 0);
    REQUIRE(buffer.written() < 512);
    REQUIRE(buffer.committed() == buffer.written());
    REQUIRE(buffer.has_nested_buffers());

    // Can't test for assertions.
    // REQUIRE_ASSERT(buffer.begin());
    // REQUIRE_ASSERT(buffer.begin<osmium::Node>());
    // REQUIRE_ASSERT(buffer.end());
    // REQUIRE_ASSERT(buffer.end<osmium::Node>());
    // REQUIRE_ASSERT(buffer.cbegin());
    // REQUIRE_ASSERT(buffer.cbegin<osmium::Node>());
    // REQUIRE_ASSERT(buffer.cend());
    // REQUIRE_ASSERT(buffer.cend<osmium::Node>());
    // REQUIRE_ASSERT(buffer.get_iterator(0));
    // REQUIRE_ASSERT(buffer.get_iterator<osmium::Node>(0));
    // REQUIRE_ASSERT(buffer.select<osmium::Node>());
}

TEST_CASE("Can populate and iterate unnested auto_grow::internal") {
    osmium::memory::Buffer buffer = make_populated_buffer(3, osmium::memory::Buffer::auto_grow::internal);
    REQUIRE(buffer);
    REQUIRE(buffer.is_aligned());
    REQUIRE(buffer.capacity() == 512);
    REQUIRE(buffer.written() > 0);
    REQUIRE(buffer.written() < 512);
    REQUIRE(buffer.committed() == buffer.written());
    REQUIRE(!buffer.has_nested_buffers());

    // REQUIRE that none of these functions raise anything:
    buffer.begin();
    buffer.begin<osmium::Node>();
    buffer.end();
    buffer.end<osmium::Node>();
    buffer.cbegin();
    buffer.cbegin<osmium::Node>();
    buffer.cend();
    buffer.cend<osmium::Node>();
    buffer.get_iterator(0);
    buffer.get_iterator<osmium::Node>(0);
    buffer.select<osmium::Node>();
}

TEST_CASE("Can iterate empty auto_grow::internal") {
    osmium::memory::Buffer buffer = make_populated_buffer(0, osmium::memory::Buffer::auto_grow::internal);
    REQUIRE(buffer);
    REQUIRE(buffer.is_aligned());
    REQUIRE(buffer.capacity() == 512);
    REQUIRE(buffer.written() == 0);
    REQUIRE(buffer.committed() == buffer.written());
    REQUIRE(!buffer.has_nested_buffers());

    // REQUIRE that none of these functions raise anything:
    buffer.begin();
    buffer.begin<osmium::Node>();
    buffer.end();
    buffer.end<osmium::Node>();
    buffer.cbegin();
    buffer.cbegin<osmium::Node>();
    buffer.cend();
    buffer.cend<osmium::Node>();
    buffer.get_iterator(0);
    buffer.get_iterator<osmium::Node>(0);
    buffer.select<osmium::Node>();
}
