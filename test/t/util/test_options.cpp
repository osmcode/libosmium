#include "catch.hpp"

#include <iterator>

#include <osmium/util/options.hpp>

TEST_CASE("Options") {

    osmium::util::Options o;

    SECTION("set_simple") {
        o.set("foo", "bar");
        REQUIRE("bar" == o.get("foo"));
        REQUIRE("" == o.get("empty"));
        REQUIRE("default" == o.get("empty", "default"));

        REQUIRE(!o.is_true("foo"));
        REQUIRE(!o.is_true("empty"));

        REQUIRE(o.is_not_false("foo"));
        REQUIRE(o.is_not_false("empty"));

        REQUIRE(1 == o.size());
    }

    SECTION("set_from_bool") {
        o.set("t", true);
        o.set("f", false);
        REQUIRE("true" == o.get("t"));
        REQUIRE("false" == o.get("f"));
        REQUIRE("" == o.get("empty"));

        REQUIRE(o.is_true("t"));
        REQUIRE(!o.is_true("f"));

        REQUIRE(o.is_not_false("t"));
        REQUIRE(!o.is_not_false("f"));

        REQUIRE(2 == o.size());
    }

    SECTION("set_from_single_string_with_equals") {
        o.set("foo=bar");
        REQUIRE("bar" == o.get("foo"));
        REQUIRE(1 == o.size());
    }

    SECTION("set_from_single_string_without_equals") {
        o.set("foo");
        REQUIRE("true" == o.get("foo"));

        REQUIRE(o.is_true("foo"));
        REQUIRE(o.is_not_false("foo"));

        REQUIRE(1 == o.size());
    }

}

