#include "catch.hpp"

#include <osmium/osm/changeset.hpp>

#include "helper.hpp"

TEST_CASE("Basic_Changeset") {

SECTION("changeset_builder") {
    osmium::memory::Buffer buffer(10 * 1000);

    osmium::Changeset& cs1 = buffer_add_changeset(buffer,
        "user",
        {{"comment", "foo"}});

    cs1.id(42)
       .created_at(100)
       .closed_at(200)
       .num_changes(7)
       .uid(9);

    REQUIRE(42 == cs1.id());
    REQUIRE(9 == cs1.uid());
    REQUIRE(7 == cs1.num_changes());
    REQUIRE(true == cs1.closed());
    REQUIRE(osmium::Timestamp(100) == cs1.created_at());
    REQUIRE(osmium::Timestamp(200) == cs1.closed_at());
    REQUIRE(1 == cs1.tags().size());
    REQUIRE(std::string("user") == cs1.user());

    osmium::Changeset& cs2 = buffer_add_changeset(buffer,
        "user",
        {{"comment", "foo"}, {"foo", "bar"}});

    cs2.id(43)
       .created_at(120)
       .num_changes(21)
       .uid(9);

    REQUIRE(43 == cs2.id());
    REQUIRE(9 == cs2.uid());
    REQUIRE(21 == cs2.num_changes());
    REQUIRE(false == cs2.closed());
    REQUIRE(osmium::Timestamp(120) == cs2.created_at());
    REQUIRE(osmium::Timestamp() == cs2.closed_at());
    REQUIRE(2 == cs2.tags().size());
    REQUIRE(std::string("user") == cs2.user());

    REQUIRE(cs1 != cs2);
    REQUIRE(cs1 == cs1);

    REQUIRE(cs1 < cs2);
    REQUIRE(cs1 <= cs2);
    REQUIRE(false == (cs1 > cs2));
    REQUIRE(false == (cs1 >= cs2));
}

}
