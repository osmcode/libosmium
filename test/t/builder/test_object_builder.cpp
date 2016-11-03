
#include "catch.hpp"

#include <osmium/builder/osm_object_builder.hpp>
#include <osmium/memory/buffer.hpp>
#include <osmium/osm.hpp>

TEST_CASE("create node") {
    osmium::memory::Buffer buffer(1024*10);

    SECTION("complete node with tags") {
        osmium::Location loc{1.2, 3.4};

        {
            osmium::builder::NodeBuilder builder(buffer);

            builder.set_id(17);
            builder.set_visible(true);
            builder.set_version(1);
            builder.set_changeset(123);
            builder.set_uid(555);
            builder.set_timestamp("2015-07-01T00:00:01Z");
            builder.set_location(loc);

            builder.add_user("foo");

            builder.add_tags({{"highway", "primary"}, {"oneway", "yes"}});
        }

        const auto& node = buffer.get<osmium::Node>(buffer.commit());

        REQUIRE(node.id() == 17);
        REQUIRE(node.version() == 1);
        REQUIRE(node.changeset() == 123);
        REQUIRE(node.uid() == 555);
        REQUIRE(node.timestamp() == osmium::Timestamp{"2015-07-01T00:00:01Z"});
        REQUIRE(node.location() == loc);

        REQUIRE(std::string{node.user()} == "foo");

        REQUIRE(node.tags().size() == 2);
    }

    SECTION("complete way with tags") {
        {
            osmium::builder::WayBuilder builder(buffer);

            builder.set_id(17);
            builder.set_visible(true);
            builder.set_version(1);
            builder.set_changeset(123);
            builder.set_uid(555);
            builder.set_timestamp("2015-07-01T00:00:01Z");

            builder.add_user("foo");

            builder.add_tags({{"highway", "primary"}, {"oneway", "yes"}});
        }

        const auto& way = buffer.get<osmium::Way>(buffer.commit());

        REQUIRE(way.id() == 17);
        REQUIRE(way.version() == 1);
        REQUIRE(way.changeset() == 123);
        REQUIRE(way.uid() == 555);
        REQUIRE(way.timestamp() == osmium::Timestamp{"2015-07-01T00:00:01Z"});

        REQUIRE(std::string{way.user()} == "foo");

        REQUIRE(way.tags().size() == 2);
    }

    SECTION("complete relation with tags") {
        {
            osmium::builder::RelationBuilder builder(buffer);

            builder.set_id(17);
            builder.set_visible(true);
            builder.set_version(1);
            builder.set_changeset(123);
            builder.set_uid(555);
            builder.set_timestamp("2015-07-01T00:00:01Z");

            builder.add_user("foo");

            builder.add_tags({{"highway", "primary"}, {"oneway", "yes"}});
        }

        const auto& relation = buffer.get<osmium::Relation>(buffer.commit());

        REQUIRE(relation.id() == 17);
        REQUIRE(relation.version() == 1);
        REQUIRE(relation.changeset() == 123);
        REQUIRE(relation.uid() == 555);
        REQUIRE(relation.timestamp() == osmium::Timestamp{"2015-07-01T00:00:01Z"});

        REQUIRE(std::string{relation.user()} == "foo");

        REQUIRE(relation.tags().size() == 2);
    }

    SECTION("complete changeset with tags") {
        osmium::Location bl{-1.2, -3.4};
        osmium::Location tr{1.2, 3.4};

        std::string user;
        SECTION("user length small") {
            user = "foo";
        }
        SECTION("user length 7") {
            user = "1234567";
        }
        SECTION("user length 8") {
            user = "12345678";
        }
        SECTION("user length 9") {
            user = "123456789";
        }
        SECTION("user length large") {
            user = "very long user name";
        }

        {
            osmium::builder::ChangesetBuilder builder(buffer);

            builder.set_id(17);
            builder.set_uid(222);
            builder.set_created_at(osmium::Timestamp{"2016-07-03T01:23:45Z"});
            builder.set_closed_at(osmium::Timestamp{"2016-07-03T01:23:48Z"});
            builder.set_num_changes(3);
            builder.set_num_comments(2);
            builder.bounds() = osmium::Box{bl, tr};

            builder.add_user(user);
        }

        const auto& changeset = buffer.get<osmium::Changeset>(buffer.commit());

        REQUIRE(changeset.id() == 17);
        REQUIRE(changeset.uid() == 222);
        REQUIRE(changeset.created_at() == osmium::Timestamp{"2016-07-03T01:23:45Z"});
        REQUIRE(changeset.closed_at() == osmium::Timestamp{"2016-07-03T01:23:48Z"});
        REQUIRE(changeset.num_changes() == 3);
        REQUIRE(changeset.num_comments() == 2);

        const auto& box = changeset.bounds();
        REQUIRE(box.bottom_left() == bl);
        REQUIRE(box.top_right() == tr);

        REQUIRE(user == changeset.user());
    }

}

