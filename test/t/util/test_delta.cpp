#include "catch.hpp"

#include <osmium/util/delta.hpp>

TEST_CASE("delta encode") {

    osmium::util::DeltaEncode<int> x;

    SECTION("int") {
        REQUIRE(x.update(17) == 17);
        REQUIRE(x.update(10) == -7);
    }

}

TEST_CASE("delta decode") {

    osmium::util::DeltaDecode<int> x;

    SECTION("int") {
        REQUIRE(x.update(17) == 17);
        REQUIRE(x.update(10) == 27);
    }

}

TEST_CASE("delta encode and decode") {

    std::vector<int> a = { 5, -9, 22, 13, 0, 23 };

    osmium::util::DeltaEncode<int> de;
    std::vector<int> b;
    for (int x : a) {
        b.push_back(de.update(x));
    }

    osmium::util::DeltaDecode<int> dd;
    std::vector<int> c;
    for (int x : b) {
        c.push_back(dd.update(x));
    }

}

