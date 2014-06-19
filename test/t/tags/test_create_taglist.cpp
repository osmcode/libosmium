#ifdef STAND_ALONE
# define BOOST_TEST_MODULE Main
#endif
#include <boost/test/unit_test.hpp>

#include <osmium/memory/buffer.hpp>
#include <osmium/osm/tag.hpp>
#include <osmium/osm/ostream.hpp>
#include <osmium/tags/taglist.hpp>

BOOST_AUTO_TEST_SUITE(tag_list)

BOOST_AUTO_TEST_CASE(can_be_created_from_initializer_list) {
    osmium::memory::Buffer buffer(10240);

    const osmium::TagList& tl = osmium::tags::create_tag_list(buffer, {
        { "highway", "primary" },
        { "name", "Main Street" },
        { "source", "GPS" }
    });

    BOOST_CHECK_EQUAL(osmium::item_type::tag_list, tl.type());
    BOOST_CHECK_EQUAL(3, tl.size());
    BOOST_CHECK(!strcmp("highway", tl.begin()->key()));
    BOOST_CHECK(!strcmp("primary", tl.begin()->value()));
}

BOOST_AUTO_TEST_CASE(can_be_created_from_map) {
    osmium::memory::Buffer buffer(10240);

    const osmium::TagList& tl = osmium::tags::create_tag_list(buffer, std::map<const char*, const char*>({
        { "highway", "primary" },
        { "name", "Main Street" }
    }));

    BOOST_CHECK_EQUAL(osmium::item_type::tag_list, tl.type());
    BOOST_CHECK_EQUAL(2, tl.size());
    BOOST_CHECK(!strcmp("name", std::next(tl.begin(), 1)->key()));
    BOOST_CHECK(!strcmp("Main Street", std::next(tl.begin(), 1)->value()));
}

BOOST_AUTO_TEST_CASE(can_be_created_from_callback) {
    osmium::memory::Buffer buffer(10240);

    const osmium::TagList& tl = osmium::tags::create_tag_list(buffer, [](osmium::builder::TagListBuilder& tlb) {
        tlb.add_tag("highway", "primary");
        tlb.add_tag("bridge", "true");
    });

    BOOST_CHECK_EQUAL(osmium::item_type::tag_list, tl.type());
    BOOST_CHECK_EQUAL(2, tl.size());
    BOOST_CHECK(!strcmp("bridge", std::next(tl.begin(), 1)->key()));
    BOOST_CHECK(!strcmp("true", std::next(tl.begin(), 1)->value()));
}

BOOST_AUTO_TEST_CASE(can_get_tag_with_get_value_by_key) {
    osmium::memory::Buffer buffer(10240);

    const osmium::TagList& tl = osmium::tags::create_tag_list(buffer, [](osmium::builder::TagListBuilder& tlb) {
        tlb.add_tag("highway", "primary");
        tlb.add_tag("bridge", "true");
    });

    BOOST_CHECK(!strcmp("primary", tl.get_value_by_key("highway")));
    BOOST_CHECK(nullptr == tl.get_value_by_key("name"));
    BOOST_CHECK(!strcmp("foo", tl.get_value_by_key("name", "foo")));

    BOOST_CHECK(!strcmp("true", tl["bridge"]));
}

BOOST_AUTO_TEST_SUITE_END()
