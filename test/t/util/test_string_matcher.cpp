#include "catch.hpp"

#include <osmium/util/string_matcher.hpp>

#include <regex>
#include <sstream>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

static_assert(std::is_default_constructible<osmium::StringMatcher>::value, "StringMatcher should be default constructible");
static_assert(std::is_copy_constructible<osmium::StringMatcher>::value, "StringMatcher should be copy constructible");
static_assert(std::is_move_constructible<osmium::StringMatcher>::value, "StringMatcher should be move constructible");
static_assert(std::is_copy_assignable<osmium::StringMatcher>::value, "StringMatcher should be copyable");
static_assert(std::is_move_assignable<osmium::StringMatcher>::value, "StringMatcher should be moveable");

namespace {

template <typename T>
std::string print(const T& matcher) {
    std::stringstream ss;
    ss << matcher;
    return ss.str();
}

} // anonymous namespace

TEST_CASE("String matcher: always false") {
    const osmium::StringMatcher::always_false m;
    REQUIRE_FALSE(m.match("foo"));
}

TEST_CASE("String matcher: always true") {
    const osmium::StringMatcher::always_true m;
    REQUIRE(m.match("foo"));
}

TEST_CASE("String matcher: equal") {
    const osmium::StringMatcher::equal m{"foo"};
    REQUIRE(m.match("foo"));
    REQUIRE_FALSE(m.match("bar"));
    REQUIRE_FALSE(m.match("foobar"));
}

TEST_CASE("String matcher: prefix from const char*") {
    const osmium::StringMatcher::prefix m{"foo"};
    REQUIRE(m.match("foo"));
    REQUIRE_FALSE(m.match("bar"));
    REQUIRE(m.match("foobar"));
    REQUIRE_FALSE(m.match(""));
}

TEST_CASE("String matcher: prefix from std::string") {
    const std::string v{"foo"};
    const osmium::StringMatcher::prefix m{v};
    REQUIRE(m.match("foo"));
    REQUIRE_FALSE(m.match("bar"));
    REQUIRE(m.match("foobar"));
    REQUIRE_FALSE(m.match(""));
}

TEST_CASE("String matcher: substring from const char*") {
    const osmium::StringMatcher::substring m{"foo"};
    REQUIRE(m.match("foo"));
    REQUIRE_FALSE(m.match("bar"));
    REQUIRE(m.match("foobar"));
    REQUIRE(m.match("barfoo"));
    REQUIRE(m.match("xfoox"));
}

TEST_CASE("String matcher: substring from std::string") {
    const std::string v{"foo"};
    const osmium::StringMatcher::substring m{v};
    REQUIRE(m.match("foo"));
    REQUIRE_FALSE(m.match("bar"));
    REQUIRE(m.match("foobar"));
    REQUIRE(m.match("barfoo"));
    REQUIRE(m.match("xfoox"));
}

TEST_CASE("String matcher: empty prefix") {
    const osmium::StringMatcher::prefix m{""};
    REQUIRE(m.match("foo"));
    REQUIRE(m.match("bar"));
    REQUIRE(m.match("foobar"));
    REQUIRE(m.match(""));
}

TEST_CASE("String matcher: regex prefix") {
    const osmium::StringMatcher::regex m{std::regex{"^foo", std::regex::optimize}};
    REQUIRE(m.match("foo"));
    REQUIRE_FALSE(m.match("bar"));
    REQUIRE(m.match("foobar"));
    REQUIRE_FALSE(m.match(""));
}

TEST_CASE("String matcher: regex substr") {
    const osmium::StringMatcher::regex m{std::regex{"foo", std::regex::optimize}};
    REQUIRE(m.match("foo"));
    REQUIRE_FALSE(m.match("bar"));
    REQUIRE(m.match("foobar"));
    REQUIRE(m.match("xfoox"));
    REQUIRE_FALSE(m.match(""));
}

TEST_CASE("String matcher: list") {
    const osmium::StringMatcher::list m{{"foo", "bar"}};
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
    m.add_string(std::string{"bar"});
    REQUIRE(m.match("bar"));
    REQUIRE_FALSE(m.match("foobar"));
    REQUIRE_FALSE(m.match("baz"));
    REQUIRE_FALSE(m.match(""));
}

TEST_CASE("Default constructed StringMatcher matches nothing") {
    const osmium::StringMatcher m;
    REQUIRE_FALSE(m("foo"));
    REQUIRE_FALSE(m("bar"));
    REQUIRE(print(m) == "always_false");
}

TEST_CASE("Construct StringMatcher from bool") {
    const osmium::StringMatcher m1{false};
    REQUIRE_FALSE(m1("foo"));
    REQUIRE_FALSE(m1("bar"));
    REQUIRE(print(m1) == "always_false");

    const osmium::StringMatcher m2{true};
    REQUIRE(m2("foo"));
    REQUIRE(m2("bar"));
    REQUIRE(print(m2) == "always_true");
}

TEST_CASE("Construct StringMatcher from string") {
    const osmium::StringMatcher m{"foo"};
    REQUIRE(m("foo"));
    REQUIRE_FALSE(m("bar"));
    REQUIRE(print(m) == "equal[foo]");
}

TEST_CASE("Construct StringMatcher from regex") {
    const osmium::StringMatcher m{std::regex{"^foo"}};
    REQUIRE(m("foo"));
    REQUIRE_FALSE(m("bar"));
    REQUIRE(print(m) == "regex");
}

TEST_CASE("Construct StringMatcher from list") {
    const std::vector<std::string> v{"foo", "xxx"};
    const osmium::StringMatcher m{v};
    REQUIRE(m("foo"));
    REQUIRE_FALSE(m("bar"));
    REQUIRE(print(m) == "list[[foo][xxx]]");
}

TEST_CASE("Construct StringMatcher") {
    osmium::StringMatcher m{osmium::StringMatcher::equal{"foo"}};
    REQUIRE(print(m) == "equal[foo]");
    REQUIRE(m("foo"));
    REQUIRE_FALSE(m("bar"));

    m = osmium::StringMatcher::list{{"foo", "bar"}};
    REQUIRE(m("foo"));
    REQUIRE(m(std::string{"bar"}));
    REQUIRE(print(m) == "list[[foo][bar]]");

    m = osmium::StringMatcher::prefix{"foo"};
    REQUIRE(m("foo"));
    REQUIRE(m("foobar"));
    REQUIRE_FALSE(m("barfoo"));
    REQUIRE(print(m) == "prefix[foo]");

    m = osmium::StringMatcher::substring{"foo"};
    REQUIRE(m("foo"));
    REQUIRE(m("foobar"));
    REQUIRE(m(std::string{"barfoo"}));
    REQUIRE(print(m) == "substring[foo]");
}

TEST_CASE("Copy construct StringMatcher") {
    const osmium::StringMatcher m1{"foo"};
    const osmium::StringMatcher m2{m1}; // NOLINT(performance-unnecessary-copy-initialization)

    REQUIRE(print(m1) == "equal[foo]");
    REQUIRE(print(m2) == "equal[foo]");
}

TEST_CASE("Copy assign StringMatcher") {
    const osmium::StringMatcher m1{"foo"};
    osmium::StringMatcher m2{"bar"};
    m2 = m1;

    REQUIRE(print(m1) == "equal[foo]");
    REQUIRE(print(m2) == "equal[foo]");
}

TEST_CASE("Move construct StringMatcher") {
    osmium::StringMatcher m1{"foo"};
    const osmium::StringMatcher m2{std::move(m1)};

    REQUIRE(print(m2) == "equal[foo]");
}

TEST_CASE("Move assign StringMatcher") {
    osmium::StringMatcher m1{"foo"};
    osmium::StringMatcher m2{"bar"};
    m2 = std::move(m1);

    REQUIRE(print(m2) == "equal[foo]");
}

