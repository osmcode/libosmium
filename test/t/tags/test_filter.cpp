#ifdef STAND_ALONE
# define BOOST_TEST_MODULE Main
#endif
#include <boost/test/unit_test.hpp>

#include <algorithm>

#include <osmium/builder/builder_helper.hpp>
#include <osmium/memory/buffer.hpp>
#include <osmium/osm/tag.hpp>
#include <osmium/tags/taglist.hpp>
#include <osmium/tags/filter.hpp>
#include <osmium/tags/regex_filter.hpp>

BOOST_AUTO_TEST_SUITE(Filter)

template <class TFilter>
void check_filter(const osmium::TagList& tag_list, const TFilter filter, const std::vector<bool>& reference) {
    std::vector<bool> results;
    std::transform(tag_list.begin(), tag_list.end(), std::back_inserter(results), filter);
    BOOST_CHECK_EQUAL_COLLECTIONS(results.begin(), results.end(), reference.begin(), reference.end());
}

BOOST_AUTO_TEST_CASE(KeyFilter_matches_some_tags) {
    osmium::tags::KeyFilter filter(false);
    filter.add(true, "highway").add(true, "railway");

    osmium::memory::Buffer buffer(10240);
    const osmium::TagList& tag_list = osmium::builder::build_tag_list(buffer, {
        { "highway", "primary" },  // match
        { "name", "Main Street" }, // no match
        { "source", "GPS" }        // no match
    });

    std::vector<bool> results = { true, false, false };

    check_filter(tag_list, filter, results);
}

BOOST_AUTO_TEST_CASE(KeyFilter_iterator_filters_tags) {
    osmium::tags::KeyFilter filter(false);
    filter.add(true, "highway").add(true, "source");

    osmium::memory::Buffer buffer(10240);
    const osmium::TagList& tl = osmium::builder::build_tag_list(buffer, {
        { "highway", "primary" },  // match
        { "name", "Main Street" }, // no match
        { "source", "GPS" }        // no match
    });

    osmium::tags::KeyFilter::iterator it(filter, tl.begin(), tl.end());
    const osmium::tags::KeyFilter::iterator end(filter, tl.end(), tl.end());

    BOOST_CHECK_EQUAL(2, std::distance(it, end));

    BOOST_CHECK(it != end);
    BOOST_CHECK_EQUAL("highway", it->key());
    BOOST_CHECK_EQUAL("primary", it->value());
    ++it;
    BOOST_CHECK_EQUAL("source", it->key());
    BOOST_CHECK_EQUAL("GPS", it->value());
    BOOST_CHECK(++it == end);
}

BOOST_AUTO_TEST_CASE(KeyValueFilter_matches_some_tags) {
    osmium::tags::KeyValueFilter filter(false);

    filter.add(true, "highway", "residential").add(true, "highway", "primary").add(true, "railway");

    osmium::memory::Buffer buffer(10240);
    const osmium::TagList& tag_list = osmium::builder::build_tag_list(buffer, {
        { "highway", "primary" },
        { "railway", "tram" },
        { "source", "GPS" }
    });

    std::vector<bool> results = {true, true, false};

    check_filter(tag_list, filter, results);
}

BOOST_AUTO_TEST_CASE(KeyValueFilter_ordering_matters) {
    osmium::tags::KeyValueFilter filter1(false);
    filter1.add(true, "highway").add(false, "highway", "road");

    osmium::tags::KeyValueFilter filter2(false);
    filter2.add(false, "highway", "road").add(true, "highway");

    osmium::memory::Buffer buffer(10240);

    const osmium::TagList& tag_list1 = osmium::builder::build_tag_list(buffer, {
        { "highway", "road" },
        { "name", "Main Street" }
    });

    const osmium::TagList& tag_list2 = osmium::builder::build_tag_list(buffer, {
        { "highway", "primary" },
        { "name", "Main Street" }
    });

    check_filter(tag_list1, filter1, {true, false});
    check_filter(tag_list1, filter2, {false, false});
    check_filter(tag_list2, filter2, {true, false});
}

BOOST_AUTO_TEST_CASE(KeyValueFilter_matches_against_taglist_with_any) {
    osmium::tags::KeyValueFilter filter(false);

    filter.add(true, "highway", "primary").add(true, "name");

    osmium::memory::Buffer buffer(10240);
    const osmium::TagList& tag_list = osmium::builder::build_tag_list(buffer, {
        { "highway", "primary" },
        { "railway", "tram" },
        { "source", "GPS" }
    });

    BOOST_CHECK( osmium::tags::match_any_of(tag_list, filter));
    BOOST_CHECK(!osmium::tags::match_all_of(tag_list, filter));
    BOOST_CHECK(!osmium::tags::match_none_of(tag_list, filter));
}

BOOST_AUTO_TEST_CASE(KeyValueFilter_matches_against_taglist_with_all) {
    osmium::tags::KeyValueFilter filter(false);

    filter.add(true, "highway", "primary").add(true, "name");

    osmium::memory::Buffer buffer(10240);
    const osmium::TagList& tag_list = osmium::builder::build_tag_list(buffer, {
        { "highway", "primary" },
        { "name", "Main Street" }
    });

    BOOST_CHECK( osmium::tags::match_any_of(tag_list, filter));
    BOOST_CHECK( osmium::tags::match_all_of(tag_list, filter));
    BOOST_CHECK(!osmium::tags::match_none_of(tag_list, filter));
}

BOOST_AUTO_TEST_CASE(KeyValueFilter_matches_against_taglist_with_none) {
    osmium::tags::KeyValueFilter filter(false);

    filter.add(true, "highway", "road").add(true, "source");

    osmium::memory::Buffer buffer(10240);
    const osmium::TagList& tag_list = osmium::builder::build_tag_list(buffer, {
        { "highway", "primary" },
        { "name", "Main Street" }
    });

    BOOST_CHECK(!osmium::tags::match_any_of(tag_list, filter));
    BOOST_CHECK(!osmium::tags::match_all_of(tag_list, filter));
    BOOST_CHECK( osmium::tags::match_none_of(tag_list, filter));
}

BOOST_AUTO_TEST_CASE(KeyValueFilter_matches_against_taglist_with_any_called_with_rvalue) {
    osmium::memory::Buffer buffer(10240);
    const osmium::TagList& tag_list = osmium::builder::build_tag_list(buffer, {
        { "highway", "primary" },
        { "railway", "tram" },
        { "source", "GPS" }
    });

    BOOST_CHECK(osmium::tags::match_any_of(tag_list,
        osmium::tags::KeyValueFilter().add(true, "highway", "primary").add(true, "name")));
}

BOOST_AUTO_TEST_CASE(RegexFilter_matches_some_tags) {
    osmium::tags::RegexFilter filter(false);
    filter.add(true, "highway", std::regex(".*_link"));

    osmium::memory::Buffer buffer(10240);
    const osmium::TagList& tag_list1 = osmium::builder::build_tag_list(buffer, {
        { "highway", "primary_link" },
        { "source", "GPS" }
    });
    const osmium::TagList& tag_list2 = osmium::builder::build_tag_list(buffer, {
        { "highway", "primary" },
        { "source", "GPS" }
    });

    check_filter(tag_list1, filter, {true, false});
    check_filter(tag_list2, filter, {false, false});
}

BOOST_AUTO_TEST_CASE(RegexFilter_matches_some_tags_with_lvalue_regex) {
    osmium::tags::RegexFilter filter(false);
    std::regex r(".*straße");
    filter.add(true, "name", r);

    osmium::memory::Buffer buffer(10240);
    const osmium::TagList& tag_list = osmium::builder::build_tag_list(buffer, {
        { "highway", "primary" },
        { "name", "Hauptstraße" }
    });

    check_filter(tag_list, filter, {false, true});
}

BOOST_AUTO_TEST_CASE(KeyPrefixFilter_matches_some_tags) {
    osmium::tags::KeyPrefixFilter filter(false);
    filter.add(true, "name:");

    osmium::memory::Buffer buffer(10240);
    const osmium::TagList& tag_list = osmium::builder::build_tag_list(buffer, {
        { "highway", "primary" },
        { "name:de", "Hauptstraße" }
    });

    check_filter(tag_list, filter, {false, true});
}

BOOST_AUTO_TEST_SUITE_END()
