#include "catch.hpp"

#include <osmium/util/string_matcher.hpp>

TEST_CASE("String matcher: always false") {
    osmium::StringMatcher::always_false m;
    REQUIRE_FALSE(m.match("foo"));
}

TEST_CASE("String matcher: always true") {
    osmium::StringMatcher::always_true m;
    REQUIRE(m.match("foo"));
}

TEST_CASE("String matcher: equal") {
    osmium::StringMatcher::equal m{"foo"};
    REQUIRE(m.match("foo"));
    REQUIRE_FALSE(m.match("bar"));
    REQUIRE_FALSE(m.match("foobar"));
}

TEST_CASE("String matcher: prefix") {
    osmium::StringMatcher::prefix m{"foo"};
    REQUIRE(m.match("foo"));
    REQUIRE_FALSE(m.match("bar"));
    REQUIRE(m.match("foobar"));
    REQUIRE_FALSE(m.match(""));
}

TEST_CASE("String matcher: substring") {
    osmium::StringMatcher::substring m{"foo"};
    REQUIRE(m.match("foo"));
    REQUIRE_FALSE(m.match("bar"));
    REQUIRE(m.match("foobar"));
    REQUIRE(m.match("barfoo"));
    REQUIRE(m.match("xfoox"));
}

TEST_CASE("String matcher: empty prefix") {
    osmium::StringMatcher::prefix m{""};
    REQUIRE(m.match("foo"));
    REQUIRE(m.match("bar"));
    REQUIRE(m.match("foobar"));
    REQUIRE(m.match(""));
}

TEST_CASE("String matcher: regex prefix") {
    osmium::StringMatcher::regex m{std::regex{"^foo", std::regex::optimize}};
    REQUIRE(m.match("foo"));
    REQUIRE_FALSE(m.match("bar"));
    REQUIRE(m.match("foobar"));
    REQUIRE_FALSE(m.match(""));
}

TEST_CASE("String matcher: regex substr") {
    osmium::StringMatcher::regex m{std::regex{"foo", std::regex::optimize}};
    REQUIRE(m.match("foo"));
    REQUIRE_FALSE(m.match("bar"));
    REQUIRE(m.match("foobar"));
    REQUIRE(m.match("xfoox"));
    REQUIRE_FALSE(m.match(""));
}

TEST_CASE("String matcher: list") {
    osmium::StringMatcher::list m{{"foo", "bar"}};
    REQUIRE(m.match("foo"));
    REQUIRE(m.match("bar"));
    REQUIRE_FALSE(m.match("foobar"));
    REQUIRE_FALSE(m.match("baz"));
    REQUIRE_FALSE(m.match(""));
}

TEST_CASE("String matcher: list with add") {
    osmium::StringMatcher::list m;
    REQUIRE_FALSE(m.match("foo"));
    REQUIRE_FALSE(m.match("bar"));
    m.add_string("foo");
    REQUIRE(m.match("foo"));
    m.add_string("bar");
    REQUIRE(m.match("bar"));
    REQUIRE_FALSE(m.match("foobar"));
    REQUIRE_FALSE(m.match("baz"));
    REQUIRE_FALSE(m.match(""));
}

TEST_CASE("Default constructed StringMatcher matches nothing") {
    osmium::StringMatcher m;
    REQUIRE_FALSE(m("foo"));
    REQUIRE_FALSE(m("bar"));
}

TEST_CASE("Construct StringMatcher from bool") {
    osmium::StringMatcher m1{false};
    REQUIRE_FALSE(m1("foo"));
    REQUIRE_FALSE(m1("bar"));

    osmium::StringMatcher m2{true};
    REQUIRE(m2("foo"));
    REQUIRE(m2("bar"));
}

TEST_CASE("Construct StringMatcher from string") {
    osmium::StringMatcher m{"foo"};
    REQUIRE(m("foo"));
    REQUIRE_FALSE(m("bar"));
}

TEST_CASE("Construct StringMatcher from regex") {
    osmium::StringMatcher m{std::regex{"^foo"}};
    REQUIRE(m("foo"));
    REQUIRE_FALSE(m("bar"));
}

TEST_CASE("Construct StringMatcher from list") {
    std::vector<std::string> v{"foo", "xxx"};
    osmium::StringMatcher m{v};
    REQUIRE(m("foo"));
    REQUIRE_FALSE(m("bar"));
}

TEST_CASE("Construct StringMatcher") {
    osmium::StringMatcher m{osmium::StringMatcher::equal{"foo"}};
    REQUIRE(m("foo"));
    REQUIRE_FALSE(m("bar"));

    m = osmium::StringMatcher::list{{"foo", "bar"}};
    REQUIRE(m("foo"));
    REQUIRE(m("bar"));
}

