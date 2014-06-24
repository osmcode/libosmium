#ifdef STAND_ALONE
# define BOOST_TEST_MODULE Main
#endif
#include <boost/test/unit_test.hpp>

#include <osmium/builder/builder_helper.hpp>
#include <osmium/memory/buffer.hpp>
#include <osmium/osm/item_type_ostream.hpp>
#include <osmium/osm/tag.hpp>

BOOST_AUTO_TEST_SUITE(tag_list)

BOOST_AUTO_TEST_CASE(can_be_created_from_initializer_list) {
    osmium::memory::Buffer buffer(10240);

    const osmium::TagList& tl = osmium::builder::build_tag_list(buffer, {
        { "highway", "primary" },
        { "name", "Main Street" },
        { "source", "GPS" }
    });

    BOOST_CHECK_EQUAL(osmium::item_type::tag_list, tl.type());
    BOOST_CHECK_EQUAL(3, tl.size());
    BOOST_CHECK_EQUAL(std::string("highway"), tl.begin()->key());
    BOOST_CHECK_EQUAL(std::string("primary"), tl.begin()->value());
}

BOOST_AUTO_TEST_CASE(can_be_created_from_map) {
    osmium::memory::Buffer buffer(10240);

    const osmium::TagList& tl = osmium::builder::build_tag_list(buffer, std::map<const char*, const char*>({
        { "highway", "primary" },
        { "name", "Main Street" }
    }));

    BOOST_CHECK_EQUAL(osmium::item_type::tag_list, tl.type());
    BOOST_CHECK_EQUAL(2, tl.size());
    BOOST_CHECK_EQUAL(std::string("name"), std::next(tl.begin(), 1)->key());
    BOOST_CHECK_EQUAL(std::string("Main Street"), std::next(tl.begin(), 1)->value());
}

BOOST_AUTO_TEST_CASE(can_be_created_with_callback) {
    osmium::memory::Buffer buffer(10240);

    const osmium::TagList& tl = osmium::builder::build_tag_list(buffer, [](osmium::builder::TagListBuilder& tlb) {
        tlb.add_tag("highway", "primary");
        tlb.add_tag("bridge", "true");
    });

    BOOST_CHECK_EQUAL(osmium::item_type::tag_list, tl.type());
    BOOST_CHECK_EQUAL(2, tl.size());
    BOOST_CHECK_EQUAL(std::string("bridge"), std::next(tl.begin(), 1)->key());
    BOOST_CHECK_EQUAL(std::string("true"), std::next(tl.begin(), 1)->value());
}

BOOST_AUTO_TEST_CASE(returns_value_by_key) {
    osmium::memory::Buffer buffer(10240);

    const osmium::TagList& tl = osmium::builder::build_tag_list(buffer, [](osmium::builder::TagListBuilder& tlb) {
        tlb.add_tag("highway", "primary");
        tlb.add_tag("bridge", "true");
    });

    BOOST_CHECK_EQUAL(std::string("primary"), tl.get_value_by_key("highway"));
    BOOST_CHECK(nullptr == tl.get_value_by_key("name"));
    BOOST_CHECK_EQUAL(std::string("foo"), tl.get_value_by_key("name", "foo"));

    BOOST_CHECK_EQUAL(std::string("true"), tl["bridge"]);
}

BOOST_AUTO_TEST_SUITE_END()
