/* The code in this file is released into the Public Domain. */

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <osmium/io/detail/queue_util.hpp>
#include <osmium/io/gzip_compression.hpp>
#include <osmium/io/xml_input.hpp>
#include <osmium/util/misc.hpp>

#include <atomic>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <future>
#include <iterator>
#include <stdexcept>
#include <string>
#include <utility>

namespace {

std::string S_(const char* s) {
    return std::string{s};
}

// From C++20 we need to handle unicode literals differently
#ifdef __cpp_char8_t
std::string S_(const char8_t* s) {
    return std::string{reinterpret_cast<const char*>(s)};
}
#endif

std::string filename(const char* test_id, const char* suffix = "osm") {
    const char* testdir = getenv("TESTDIR");
    if (!testdir) {
        throw std::runtime_error{"You have to set TESTDIR environment variable before running testdata-xml"};
    }

    std::string f;
    f += testdir;
    f += "/";
    f += test_id;
    f += "/data.";
    f += suffix;
    return f;
}

struct header_buffer_type {
    osmium::io::Header header;
    osmium::memory::Buffer buffer;
};

// =============================================

// The following helper functions are used to call different parts of the
// Osmium internals used to read and parse XML files. This way those parts
// can be tested individually. These function can not be used in normal
// operations, because they make certain assumptions, for instance that
// file contents fit into small buffers.

std::string read_file(const char* test_id) {
    const int fd = osmium::io::detail::open_for_reading(filename(test_id));
    assert(fd >= 0);

    std::string input(10000, '\0');
    const auto n = ::read(fd, &*input.begin(), 10000);
    assert(n >= 0);
    input.resize(static_cast<std::string::size_type>(n));

    close(fd);

    return input;
}

std::string read_gz_file(const char* test_id, const char* suffix) {
    const int fd = osmium::io::detail::open_for_reading(filename(test_id, suffix));
    assert(fd >= 0);

    osmium::io::GzipDecompressor gzip_decompressor{fd};
    std::string input = gzip_decompressor.read();
    gzip_decompressor.close();

    return input;
}

header_buffer_type parse_xml(std::string input) {
    osmium::thread::Pool pool;
    osmium::io::detail::future_string_queue_type input_queue;
    osmium::io::detail::future_buffer_queue_type output_queue;
    std::promise<osmium::io::Header> header_promise;
    std::future<osmium::io::Header> header_future = header_promise.get_future();

    osmium::io::detail::add_to_queue(input_queue, std::move(input));
    osmium::io::detail::add_to_queue(input_queue, std::string{});

    std::atomic<std::size_t> offset{0};

    osmium::io::detail::parser_arguments args = {
        pool,
        -1,
        input_queue,
        output_queue,
        header_promise,
        &offset,
        osmium::osm_entity_bits::all,
        osmium::io::read_meta::yes,
        osmium::io::buffers_type::any,
        false
    };
    osmium::io::detail::XMLParser parser{args};
    parser.parse();

    header_buffer_type result;
    result.header = header_future.get();
    std::future<osmium::memory::Buffer> future_buffer;
    output_queue.wait_and_pop(future_buffer);
    result.buffer = future_buffer.get();

    if (result.buffer) {
        std::future<osmium::memory::Buffer> future_buffer2;
        output_queue.wait_and_pop(future_buffer2);
        assert(!future_buffer2.get());
    }

    return result;
}

header_buffer_type read_xml(const char* test_id) {
    const std::string input = read_file(test_id);
    return parse_xml(input);
}

template <typename TException>
void test_fail(const char* xml_file_name, const char* errmsg) {
    REQUIRE_THROWS_AS(read_xml(xml_file_name), TException);
    REQUIRE_THROWS_WITH(read_xml(xml_file_name), errmsg);

    REQUIRE_THROWS_AS([&](){
        osmium::io::Reader reader{filename(xml_file_name)};
        const osmium::io::Header header{reader.header()};
        const osmium::memory::Buffer buffer = reader.read();
        reader.close();
    }(), TException);
}

} // anonymous namespace

// =============================================

TEST_CASE("Reading OSM XML 100: Direct") {
    const header_buffer_type r = read_xml("100-correct_but_no_data");

    REQUIRE(r.header.get("generator") == "testdata");
    REQUIRE(0 == r.buffer.committed());
    REQUIRE_FALSE(r.buffer);
}

TEST_CASE("Reading OSM XML 100: Using Reader") {
    osmium::io::Reader reader{filename("100-correct_but_no_data")};

    const osmium::io::Header header{reader.header()};
    REQUIRE(header.get("generator") == "testdata");

    const osmium::memory::Buffer buffer = reader.read();
    REQUIRE(0 == buffer.committed());
    REQUIRE_FALSE(buffer);
    reader.close();
}

TEST_CASE("Reading OSM XML 100: Using Reader asking for header only") {
    osmium::io::Reader reader{filename("100-correct_but_no_data"), osmium::osm_entity_bits::nothing};

    const osmium::io::Header header{reader.header()};
    REQUIRE(header.get("generator") == "testdata");
    reader.close();
}

// =============================================

TEST_CASE("Reading OSM XML 101") {
    test_fail<osmium::format_version_error>("101-missing_version", "Can not read file without version (missing version attribute on osm element).");

    try {
        read_xml("101-missing_version");
    } catch (const osmium::format_version_error& e) {
        REQUIRE(e.version.empty());
    }
}

// =============================================

TEST_CASE("Reading OSM XML 102") {
    test_fail<osmium::format_version_error>("102-wrong_version", "Can not read file with version 0.1");

    try {
        read_xml("102-wrong_version");
    } catch (const osmium::format_version_error& e) {
        REQUIRE(e.version == "0.1");
    }
}

// =============================================

TEST_CASE("Reading OSM XML 103") {
    test_fail<osmium::format_version_error>("103-old_version", "Can not read file with version 0.5");

    try {
        read_xml("103-old_version");
    } catch (const osmium::format_version_error& e) {
        REQUIRE(e.version == "0.5");
    }
}

// =============================================

TEST_CASE("Reading OSM XML 104") {
    test_fail<osmium::xml_error>("104-empty_file", "XML parsing error at line 1, column 0: no element found");

    try {
        read_xml("104-empty_file");
    } catch (const osmium::xml_error& e) {
        REQUIRE(e.line == 1);
        REQUIRE(e.column == 0);
    }
}

// =============================================

TEST_CASE("Reading OSM XML 105") {
    test_fail<osmium::xml_error>("105-incomplete_xml_file", "XML parsing error at line 3, column 0: no element found");
}

// =============================================

TEST_CASE("Reading OSM XML 106") {
    test_fail<osmium::xml_error>("106-invalid_xml_file", "xml file nested too deep");
}

// =============================================

TEST_CASE("Reading OSM XML 107") {
    test_fail<osmium::xml_error>("107-wrongly_nested_xml_file", "Unknown element in <node>: modify");
}

// =============================================

TEST_CASE("Reading OSM XML 108") {
    test_fail<osmium::xml_error>("108-unknown_top_level", "Unknown top-level element: foo");
}

// =============================================

TEST_CASE("Reading OSM XML 109: Direct") {
    read_xml("109-unknown_data_level");
}

TEST_CASE("Reading OSM XML 109: Using Reader") {
    osmium::io::Reader reader{filename("109-unknown_data_level")};

    const osmium::io::Header header{reader.header()};
    const osmium::memory::Buffer buffer = reader.read();
    REQUIRE(buffer.committed() > 0);
    reader.close();
}

// =============================================

TEST_CASE("Reading OSM XML 110") {
    test_fail<osmium::xml_error>("110-entity_declaration", "XML entities are not supported");
}

// =============================================

TEST_CASE("Reading OSM XML 120: Direct") {
    const std::string data = read_gz_file("120-correct_gzip_file_without_data", "osm.gz");

    REQUIRE(data.size() == 102);

    const header_buffer_type r = parse_xml(data);
    REQUIRE(r.header.get("generator") == "testdata");
    REQUIRE(0 == r.buffer.committed());
    REQUIRE_FALSE(r.buffer);
}

TEST_CASE("Reading OSM XML 120: Using Reader") {
    osmium::io::Reader reader{filename("120-correct_gzip_file_without_data", "osm.gz")};

    const osmium::io::Header header{reader.header()};
    REQUIRE(header.get("generator") == "testdata");

    const osmium::memory::Buffer buffer = reader.read();
    REQUIRE(0 == buffer.committed());
    REQUIRE_FALSE(buffer);
    reader.close();
}

// =============================================

TEST_CASE("Reading OSM XML 121: Direct") {
    REQUIRE_THROWS_AS(read_gz_file("121-truncated_gzip_file", "osm.gz"), osmium::gzip_error);
}

TEST_CASE("Reading OSM XML 121: Using Reader") {
    // can throw osmium::gzip_error or osmium::xml_error
    REQUIRE_THROWS([](){
        osmium::io::Reader reader{filename("121-truncated_gzip_file", "osm.gz")};
        const osmium::io::Header header{reader.header()};
        const osmium::memory::Buffer buffer = reader.read();
        reader.close();
    }());
}

// =============================================

TEST_CASE("Reading OSM XML 122") {
    test_fail<osmium::xml_error>("122-no_osm_element", "Unknown top-level element: node");
}

// =============================================

TEST_CASE("Reading OSM XML 123") {
    test_fail<osmium::xml_error>("123-unknown_element_in_node", "Unknown element in <node>: nd");
}

// =============================================

TEST_CASE("Reading OSM XML 124") {
    test_fail<osmium::xml_error>("124-unknown_element_in_way", "Unknown element in <way>: member");
}

// =============================================

TEST_CASE("Reading OSM XML 125") {
    test_fail<osmium::xml_error>("125-unknown_element_in_relation", "Unknown element in <relation>: nd");
}

// =============================================

TEST_CASE("Reading OSM XML 126") {
    test_fail<osmium::xml_error>("126-wrong_member_type", "Unknown type on relation <member>");
}

// =============================================

TEST_CASE("Reading OSM XML 127") {
    test_fail<osmium::xml_error>("127-missing_member_type", "Unknown type on relation <member>");
}

// =============================================

TEST_CASE("Reading OSM XML 128") {
    test_fail<osmium::xml_error>("128-missing_member_ref", "Missing ref on relation <member>");
}

// =============================================

TEST_CASE("Reading OSM XML 140: Using Reader") {
    osmium::io::Reader reader{filename("140-unicode")};
    osmium::memory::Buffer buffer = reader.read();
    reader.close();

    int count = 0;
    for (const auto& node : buffer.select<osmium::Node>()) {
        ++count;
        REQUIRE(node.id() == count);
        const osmium::TagList& t = node.tags();

        const char* uc = t["unicode_char"];

        const auto len = osmium::detail::str_to_int<std::size_t>(t["unicode_utf8_length"]);
        REQUIRE(len == std::strlen(uc));

        REQUIRE(S_(uc) == t["unicode_xml"]);

        switch (count) {
            case 1:
                REQUIRE(S_(uc) == S_(u8"a"));
                break;
            case 2:
                REQUIRE(S_(uc) == S_(u8"\u00e4"));
                break;
            case 3:
                REQUIRE(S_(uc) == S_(u8"\u30dc"));
                break;
            case 4:
                REQUIRE(S_(uc) == S_(u8"\U0001d11e"));
                break;
            case 5:
                REQUIRE(S_(uc) == S_(u8"\U0001f6eb"));
                break;
            default:
                REQUIRE(false); // should not be here
        }
    }
    REQUIRE(count == 5);
}

// =============================================

TEST_CASE("Reading OSM XML 141: Using Reader") {
    osmium::io::Reader reader{filename("141-entities")};
    const osmium::memory::Buffer buffer = reader.read();
    reader.close();
    REQUIRE(buffer.committed() > 0);
    REQUIRE(buffer.get<osmium::memory::Item>(0).type() == osmium::item_type::node);

    const osmium::Node& node = buffer.get<osmium::Node>(0);
    const osmium::TagList& tags = node.tags();

    REQUIRE(S_(tags["less-than"])    == "<");
    REQUIRE(S_(tags["greater-than"]) == ">");
    REQUIRE(S_(tags["apostrophe"])   == "'");
    REQUIRE(S_(tags["ampersand"])    == "&");
    REQUIRE(S_(tags["quote"])        == "\"");
}

// =============================================

TEST_CASE("Reading OSM XML 142: Using Reader to read nodes") {
    osmium::io::Reader reader{filename("142-whitespace")};
    const osmium::memory::Buffer buffer = reader.read();
    reader.close();

    int count = 0;
    for (const auto& node : buffer.select<osmium::Node>()) {
        ++count;
        REQUIRE(node.id() == count);
        REQUIRE(node.tags().size() == 1);
        const osmium::Tag& tag = *(node.tags().begin());

        switch (count) {
            case 1:
                REQUIRE(S_(node.user()) == "user name");
                REQUIRE(S_(tag.key()) == "key with space");
                REQUIRE(S_(tag.value()) == "value with space");
                break;
            case 2:
                REQUIRE(S_(node.user()) == "line\nfeed");
                REQUIRE(S_(tag.key()) == "key with\nlinefeed");
                REQUIRE(S_(tag.value()) == "value with\nlinefeed");
                break;
            case 3:
                REQUIRE(S_(node.user()) == "carriage\rreturn");
                REQUIRE(S_(tag.key()) == "key with\rcarriage\rreturn");
                REQUIRE(S_(tag.value()) == "value with\rcarriage\rreturn");
                break;
            case 4:
                REQUIRE(S_(node.user()) == "tab\tulator");
                REQUIRE(S_(tag.key()) == "key with\ttab");
                REQUIRE(S_(tag.value()) == "value with\ttab");
                break;
            case 5:
                REQUIRE(S_(node.user()) == "unencoded linefeed");
                REQUIRE(S_(tag.key()) == "key with unencoded linefeed");
                REQUIRE(S_(tag.value()) == "value with unencoded linefeed");
                break;
            default:
                REQUIRE(false); // should not be here
        }
    }
    REQUIRE(count == 5);
}

TEST_CASE("Reading OSM XML 142: Using Reader to read relation") {
    osmium::io::Reader reader{filename("142-whitespace")};
    const osmium::memory::Buffer buffer = reader.read();
    reader.close();

    auto it = buffer.select<osmium::Relation>().begin();
    REQUIRE(it != buffer.select<osmium::Relation>().end());
    REQUIRE(it->id() == 21);
    const auto& members = it->members();
    REQUIRE(members.size() == 5);

    int count = 0;
    for (const auto& member : members) {
        ++count;
        switch (count) {
            case 1:
                REQUIRE(S_(member.role()) == "role with whitespace");
                break;
            case 2:
                REQUIRE(S_(member.role()) == "role with\nlinefeed");
                break;
            case 3:
                REQUIRE(S_(member.role()) == "role with\rcarriage\rreturn");
                break;
            case 4:
                REQUIRE(S_(member.role()) == "role with\ttab");
                break;
            case 5:
                REQUIRE(S_(member.role()) == "role with unencoded linefeed");
                break;
            default:
                REQUIRE(false); // should not be here
        }
    }
    REQUIRE(count == 5);
}


// =============================================

TEST_CASE("Reading OSM XML 200: Direct") {
    header_buffer_type r = read_xml("200-nodes");

    REQUIRE(r.header.get("generator") == "testdata");
    REQUIRE(r.buffer.committed() > 0);
    REQUIRE(r.buffer.get<osmium::memory::Item>(0).type() == osmium::item_type::node);
    REQUIRE(r.buffer.get<osmium::Node>(0).id() == 36966060);
    REQUIRE(std::distance(r.buffer.begin(), r.buffer.end()) == 3);
}

TEST_CASE("Reading OSM XML 200: Using Reader") {
    osmium::io::Reader reader{filename("200-nodes")};

    const osmium::io::Header header{reader.header()};
    REQUIRE(header.get("generator") == "testdata");

    osmium::memory::Buffer buffer = reader.read();
    REQUIRE(buffer.committed() > 0);
    REQUIRE(buffer.get<osmium::memory::Item>(0).type() == osmium::item_type::node);
    REQUIRE(buffer.get<osmium::Node>(0).id() == 36966060);
    REQUIRE(std::distance(buffer.begin(), buffer.end()) == 3);
    reader.close();
}

TEST_CASE("Reading OSM XML 200: Using Reader asking for nodes") {
    osmium::io::Reader reader{filename("200-nodes"), osmium::osm_entity_bits::node};

    const osmium::io::Header header{reader.header()};
    REQUIRE(header.get("generator") == "testdata");

    osmium::memory::Buffer buffer = reader.read();
    REQUIRE(buffer.committed() > 0);
    REQUIRE(buffer.get<osmium::memory::Item>(0).type() == osmium::item_type::node);
    REQUIRE(buffer.get<osmium::Node>(0).id() == 36966060);
    REQUIRE(std::distance(buffer.begin(), buffer.end()) == 3);
    reader.close();
}

TEST_CASE("Reading OSM XML 200: Using Reader asking for header only") {
    osmium::io::Reader reader{filename("200-nodes"), osmium::osm_entity_bits::nothing};

    const osmium::io::Header header{reader.header()};
    REQUIRE(header.get("generator") == "testdata");

    REQUIRE_FALSE(reader.read());
    REQUIRE_THROWS(reader.read());

    reader.close();
}

TEST_CASE("Reading OSM XML 200: Using Reader asking for ways") {
    osmium::io::Reader reader{filename("200-nodes"), osmium::osm_entity_bits::way};

    const osmium::io::Header header{reader.header()};
    REQUIRE(header.get("generator") == "testdata");

    const osmium::memory::Buffer buffer = reader.read();
    REQUIRE(0 == buffer.committed());
    REQUIRE_FALSE(buffer);
    reader.close();
}

TEST_CASE("Reading OSM XML 300: Change file") {
    osmium::io::Reader reader{filename("300-change-file", "osc")};

    const osmium::io::Header header{reader.header()};
    REQUIRE(header.get("generator") == "testdata");

    osmium::memory::Buffer buffer = reader.read();

    auto it = buffer.select<osmium::Node>().begin();
    REQUIRE(it != buffer.select<osmium::Node>().end());
    REQUIRE(it->id() == 11);
    REQUIRE(it->visible());

    ++it;
    REQUIRE(it->id() == 13);
    REQUIRE_FALSE(it->visible());

    ++it;
    REQUIRE(it->id() == 14);
    REQUIRE(it->visible());

    ++it;
    REQUIRE(it == buffer.select<osmium::Node>().end());

    reader.close();
}

