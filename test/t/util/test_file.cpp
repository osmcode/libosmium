#include "catch.hpp"

#include <osmium/util/file.hpp>

TEST_CASE("pagesize") {

    SECTION("round to pagesize") {
        size_t ps = osmium::util::get_pagesize();
        REQUIRE(osmium::util::round_to_pagesize(   0) == ps);
        REQUIRE(osmium::util::round_to_pagesize(   1) == ps);
        REQUIRE(osmium::util::round_to_pagesize(ps-1) == ps);
        REQUIRE(osmium::util::round_to_pagesize(  ps) == ps);
        REQUIRE(osmium::util::round_to_pagesize(ps+1) == ps * 2);
    }

}

#ifdef _WIN32
// https://msdn.microsoft.com/en-us/library/ksazx244.aspx
// https://msdn.microsoft.com/en-us/library/a9yf33zb.aspx
class DoNothingInvalidParameterHandler {

    static void invalid_parameter_handler(
                    const wchar_t* expression,
                    const wchar_t* function,
                    const wchar_t* file,
                    unsigned int line,
                    uintptr_t pReserved
                ) {
        // do nothing
    }

    _invalid_parameter_handler old_handler;

public:

    DoNothingInvalidParameterHandler() :
        old_handler(set_invalid_parameter_handler(DoNothingInvalidParameterHandler::invalid_parameter_handler)) {
    }

    ~DoNothingInvalidParameterHandler() {
        _set_invalid_parameter_handler(old_handler);
    }

}; // class InvalidParameterHandler
#endif


TEST_CASE("file_size") {

#ifdef _WIN32
    DoNothingInvalidParameterHandler handler;
#endif

    SECTION("illegal fd should throw") {
        REQUIRE_THROWS_AS(osmium::util::file_size(-1), std::system_error);
    }

    SECTION("unused fd should throw") {
        // its unlikely that fd 1000 is open...
        REQUIRE_THROWS_AS(osmium::util::file_size(1000), std::system_error);
    }

}

TEST_CASE("resize_file") {

#ifdef _WIN32
    DoNothingInvalidParameterHandler handler;
#endif

    SECTION("illegal fd should throw") {
        REQUIRE_THROWS_AS(osmium::util::resize_file(-1, 10), std::system_error);
    }

    SECTION("unused fd should throw") {
        // its unlikely that fd 1000 is open...
        REQUIRE_THROWS_AS(osmium::util::resize_file(1000, 10), std::system_error);
    }

}

