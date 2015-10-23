
#include "catch.hpp"

#include <osmium/io/detail/output_format.hpp>

TEST_CASE("output formatted") {

    std::string out;

    SECTION("small results") {
        osmium::io::detail::output_formatted_to_string(out, "%d", 17);
        REQUIRE(out == "17");
    }

    SECTION("several parameters") {
        osmium::io::detail::output_formatted_to_string(out, "%d %s", 17, "foo");
        REQUIRE(out == "17 foo");

    }

    SECTION("string already containing something") {
        out += "foo";
        osmium::io::detail::output_formatted_to_string(out, " %d", 23);
        REQUIRE(out == "foo 23");
    }

    SECTION("large results") {
        const char* str =
            "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"
            "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"
            "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"
            "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"
            "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX";

        osmium::io::detail::output_formatted_to_string(out, "%s", str);

        REQUIRE(out == str);
    }

}

