
#include "catch.hpp"

#include <osmium/index/id_set.hpp>
#include <osmium/osm/types.hpp>

TEST_CASE("Basic functionality of IdSet") {
    osmium::index::IdSet<osmium::unsigned_object_id_type> s;

    REQUIRE_FALSE(s.get(17));
    REQUIRE_FALSE(s.get(28));
    REQUIRE(s.empty());

    s.set(17);
    REQUIRE(s.get(17));
    REQUIRE_FALSE(s.get(28));
    REQUIRE_FALSE(s.empty());

    s.set(28);
    REQUIRE(s.get(17));
    REQUIRE(s.get(28));
    REQUIRE_FALSE(s.empty());

    s.clear();
    REQUIRE(s.empty());
}

TEST_CASE("Test with larger Ids") {
    osmium::index::IdSet<osmium::unsigned_object_id_type> s;

    const osmium::unsigned_object_id_type start = 25;
    const osmium::unsigned_object_id_type end = 100000000;
    const osmium::unsigned_object_id_type step = 123456;

    for (osmium::unsigned_object_id_type i = start; i < end; i += step) {
        s.set(i);
    }

    for (osmium::unsigned_object_id_type i = start; i < end; i += step) {
        REQUIRE(s.get(i));
        REQUIRE_FALSE(s.get(i + 1));
    }
}

TEST_CASE("Large gap") {
    osmium::index::IdSet<osmium::unsigned_object_id_type> s;

    s.set(3);
    s.set(1 << 30);

    REQUIRE(s.get(1 << 30));
    REQUIRE_FALSE(s.get(1 << 29));
}

