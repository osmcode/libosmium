#include "catch.hpp"

#include <sstream>
#include <osmium/builder/attr.hpp>
#include <osmium/io/metadata_options.hpp>
#include <osmium/memory/buffer.hpp>
#include <osmium/osm/object.hpp>

TEST_CASE("Metadata options: default") {
    osmium::io::metadata_options m;
    REQUIRE_FALSE(m.none());
    REQUIRE(m.any());
    REQUIRE(m.all());
    REQUIRE(m.version());
    REQUIRE(m.timestamp());
    REQUIRE(m.changeset());
    REQUIRE(m.uid());
    REQUIRE(m.user());
}

TEST_CASE("Metadata options: false") {
    osmium::io::metadata_options m{"false"};
    REQUIRE(m.none());
    REQUIRE_FALSE(m.any());
    REQUIRE_FALSE(m.all());
    REQUIRE_FALSE(m.version());
    REQUIRE_FALSE(m.timestamp());
    REQUIRE_FALSE(m.changeset());
    REQUIRE_FALSE(m.uid());
    REQUIRE_FALSE(m.user());
}

TEST_CASE("Metadata options: none") {
    osmium::io::metadata_options m{"none"};
    REQUIRE(m.none());
    REQUIRE_FALSE(m.any());
    REQUIRE_FALSE(m.all());
    REQUIRE_FALSE(m.version());
    REQUIRE_FALSE(m.timestamp());
    REQUIRE_FALSE(m.changeset());
    REQUIRE_FALSE(m.uid());
    REQUIRE_FALSE(m.user());
}

TEST_CASE("Metadata options: true") {
    osmium::io::metadata_options m{"true"};
    REQUIRE_FALSE(m.none());
    REQUIRE(m.any());
    REQUIRE(m.all());
    REQUIRE(m.version());
    REQUIRE(m.timestamp());
    REQUIRE(m.changeset());
    REQUIRE(m.uid());
    REQUIRE(m.user());
}

TEST_CASE("Metadata options: all") {
    osmium::io::metadata_options m{"all"};
    REQUIRE_FALSE(m.none());
    REQUIRE(m.any());
    REQUIRE(m.all());
    REQUIRE(m.version());
    REQUIRE(m.timestamp());
    REQUIRE(m.changeset());
    REQUIRE(m.uid());
    REQUIRE(m.user());
}

TEST_CASE("Metadata options: version,changeset") {
    osmium::io::metadata_options m{"version+changeset"};
    REQUIRE_FALSE(m.none());
    REQUIRE(m.any());
    REQUIRE_FALSE(m.all());
    REQUIRE(m.version());
    REQUIRE_FALSE(m.timestamp());
    REQUIRE(m.changeset());
    REQUIRE_FALSE(m.uid());
    REQUIRE_FALSE(m.user());
}

TEST_CASE("Metadata options: timestamp,uid,user") {
    osmium::io::metadata_options m{"timestamp+uid+user"};
    REQUIRE_FALSE(m.none());
    REQUIRE(m.any());
    REQUIRE_FALSE(m.all());
    REQUIRE_FALSE(m.version());
    REQUIRE(m.timestamp());
    REQUIRE_FALSE(m.changeset());
    REQUIRE(m.uid());
    REQUIRE(m.user());
}

TEST_CASE("Metadata options: fail") {
    REQUIRE_THROWS_AS(osmium::io::metadata_options{"timestamp+foo"}, const std::invalid_argument&);
}

TEST_CASE("Metdata options: constructor using OSMObject") {
    osmium::memory::Buffer buffer{10 * 1000};
    using namespace osmium::builder::attr;

    SECTION("only version") {
        const osmium::OSMObject& obj = buffer.get<osmium::OSMObject>(osmium::builder::add_node(buffer, _id(1),
                _version(2)));
        osmium::io::metadata_options options = osmium::io::metadata_options_from_object(obj);
        REQUIRE_FALSE(options.user());
        REQUIRE_FALSE(options.uid());
        REQUIRE_FALSE(options.changeset());
        REQUIRE_FALSE(options.timestamp());
        REQUIRE(options.version());
    }

    SECTION("full") {
        const osmium::OSMObject& obj = buffer.get<osmium::OSMObject>(osmium::builder::add_node(buffer, _id(1),
                _version(2), _cid(30), _user("foo"), _uid(8), _timestamp("2018-01-01T23:00:00Z")));
        osmium::io::metadata_options options = osmium::io::metadata_options_from_object(obj);
        REQUIRE(options.all());
    }

    SECTION("changeset+timestamp+version") {
        const osmium::OSMObject& obj = buffer.get<osmium::OSMObject>(osmium::builder::add_node(buffer, _id(1),
                _version(2), _cid(30), _timestamp("2018-01-01T23:00:00Z")));
        osmium::io::metadata_options options = osmium::io::metadata_options_from_object(obj);
        REQUIRE(options.version());
        REQUIRE(options.timestamp());
        REQUIRE(options.changeset());
        REQUIRE_FALSE(options.user());
        REQUIRE_FALSE(options.uid());
    }
}

TEST_CASE("Metdata options: string representation should be valid") {

    SECTION("version+changeset") {
        osmium::io::metadata_options options{"version+changeset"};
        REQUIRE(options.to_string() == "version+changeset");
    }

    SECTION("version+uid+user") {
        osmium::io::metadata_options options{"version+uid+user"};
        REQUIRE(options.to_string() == "version+uid+user");
    }

    SECTION("version+timestamp") {
        osmium::io::metadata_options options{"version+timestamp"};
        REQUIRE(options.to_string() == "version+timestamp");
    }

    SECTION("timestamp+version (different order") {
        osmium::io::metadata_options options{"timestamp+version"};
        REQUIRE(options.to_string() == "version+timestamp");
    }

    SECTION("none") {
        osmium::io::metadata_options options{"none"};
        REQUIRE(options.to_string() == "none");
    }

    SECTION("all (short)") {
        osmium::io::metadata_options options{"all"};
        REQUIRE(options.to_string() == "all");
    }

    SECTION("all (long)") {
        osmium::io::metadata_options options{"user+uid+version+timestamp+changeset"};
        REQUIRE(options.to_string() == "all");
    }
}

