#include "catch.hpp"

#include <osmium/io/metadata_options.hpp>

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

