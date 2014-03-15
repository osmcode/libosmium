#ifdef STAND_ALONE
# define BOOST_TEST_MODULE Main
#endif
#include <boost/test/unit_test.hpp>

#include <osmium/memory/buffer.hpp>
#include <osmium/osm/builder.hpp>
#include <osmium/osm/tag.hpp>
#include <osmium/osm/ostream.hpp>
#include <osmium/tags/key_filter.hpp>
#include <osmium/tags/key_value_filter.hpp>

BOOST_AUTO_TEST_SUITE(Filter)

BOOST_AUTO_TEST_CASE(KeyFilter) {
    osmium::tags::KeyFilter filter(false);

    filter.add(true, "highway").add(true, "railway");

    osmium::memory::Buffer buffer(10240);
    {
        osmium::osm::TagListBuilder tl_builder(buffer);
        tl_builder.add_tag("highway", "primary");
        tl_builder.add_tag("name", "Main Street");
        tl_builder.add_tag("source", "GPS");
    }
    buffer.commit();

    const osmium::TagList& tl = buffer.get<const osmium::TagList>(0);

    auto tagit = tl.begin();
    BOOST_CHECK_EQUAL(filter(*tagit), true);
    ++tagit;
    BOOST_CHECK_EQUAL(filter(*tagit), false);

    osmium::tags::KeyFilter::iterator fi_begin(filter, tl.begin(), tl.end());
    osmium::tags::KeyFilter::iterator fi_end(filter, tl.end(), tl.end());

    BOOST_CHECK(fi_begin != fi_end);
    BOOST_CHECK_EQUAL(*tl.begin(), *fi_begin++);
    BOOST_CHECK(fi_begin == fi_end);
}

BOOST_AUTO_TEST_CASE(KeyValueFilter) {
    osmium::tags::KeyValueFilter filter(false);

    filter.add(true, "highway", "residential").add(true, "highway", "primary").add(true, "railway");

    osmium::memory::Buffer buffer(10240);
    {
        osmium::osm::TagListBuilder tl_builder(buffer);
        tl_builder.add_tag("highway", "primary");
        tl_builder.add_tag("highway", "secondary");
        tl_builder.add_tag("railway", "tram");
        tl_builder.add_tag("source", "GPS");
    }
    buffer.commit();

    const osmium::TagList& tl = buffer.get<const osmium::TagList>(0);
    const osmium::Tag& t1 = *(tl.begin());
    const osmium::Tag& t2 = *(std::next(tl.begin(), 1));
    const osmium::Tag& t3 = *(std::next(tl.begin(), 2));
    const osmium::Tag& t4 = *(std::next(tl.begin(), 3));

    BOOST_CHECK_EQUAL(filter(t1), true);
    BOOST_CHECK_EQUAL(filter(t2), false);
    BOOST_CHECK_EQUAL(filter(t3), true);
    BOOST_CHECK_EQUAL(filter(t4), false);
}

BOOST_AUTO_TEST_SUITE_END()
