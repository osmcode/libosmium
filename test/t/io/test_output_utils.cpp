
#include "catch.hpp"

#include <locale>

#include <osmium/io/detail/string_util.hpp>

TEST_CASE("output formatted") {

    std::string out;

    SECTION("small results") {
        osmium::io::detail::append_printf_formatted_string(out, "%d", 17);
        REQUIRE(out == "17");
    }

    SECTION("several parameters") {
        osmium::io::detail::append_printf_formatted_string(out, "%d %s", 17, "foo");
        REQUIRE(out == "17 foo");

    }

    SECTION("string already containing something") {
        out += "foo";
        osmium::io::detail::append_printf_formatted_string(out, " %d", 23);
        REQUIRE(out == "foo 23");
    }

    SECTION("large results") {
        const char* str =
            "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"
            "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"
            "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"
            "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"
            "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX";

        osmium::io::detail::append_printf_formatted_string(out, "%s", str);

        REQUIRE(out == str);
    }

}

TEST_CASE("UTF8 encoding") {

    std::string out;

    SECTION("append to string") {
        out += "1234";
        osmium::io::detail::append_utf8_encoded_string(out, "abc");
        REQUIRE(out == "1234abc");
    }

    SECTION("don't encode alphabetic characters") {
        const char* s = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
        osmium::io::detail::append_utf8_encoded_string(out, s);
        REQUIRE(out == s);
    }

    SECTION("don't encode numeric characters") {
        const char* s = "0123456789";
        osmium::io::detail::append_utf8_encoded_string(out, s);
        REQUIRE(out == s);
    }

    SECTION("don't encode lots of often used characters characters") {
        const char* s = ".-;:_#+";
        osmium::io::detail::append_utf8_encoded_string(out, s);
        REQUIRE(out == s);
    }

    SECTION("encode characters that are special in OPL") {
        osmium::io::detail::append_utf8_encoded_string(out, " \n,=@");
        REQUIRE(out == "%20%%0a%%2c%%3d%%40%");
    }

// workaround for missing support for u8 string literals on Windows
#if !defined(_MSC_VER)

    SECTION("encode multibyte character") {
        osmium::io::detail::append_utf8_encoded_string(out, u8"\u30dc_\U0001d11e_\U0001f6eb");
        REQUIRE(out == "%30dc%_%1d11e%_%1f6eb%");
    }

#endif

}

TEST_CASE("encoding of first 127 characters") {

    std::locale cloc("C");
    char s[] = "a\0";

    for (char c = 1; c < 0x7f; ++c) {
        std::string out;
        s[0] = c;
        osmium::io::detail::append_utf8_encoded_string(out, s);

        if (!std::isprint(c, cloc)) {
            REQUIRE(out[0] == '%');
        }
    }

}

TEST_CASE("html encoding") {

    std::string out;

    SECTION("do not encode normal characters") {
        const char* s = "abc123,.-";
        osmium::io::detail::append_xml_encoded_string(out, s);
        REQUIRE(out == s);
    }

    SECTION("encode special XML characters") {
        const char* s = "& \" \' < > \n \r \t";
        osmium::io::detail::append_xml_encoded_string(out, s);
        REQUIRE(out == "&amp; &quot; &apos; &lt; &gt; &#xA; &#xD; &#x9;");
    }

}

