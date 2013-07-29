#ifdef STAND_ALONE
# define BOOST_TEST_MODULE Main
#endif
#include <boost/test/unit_test.hpp>

#include <osmium/osm/builder.hpp>
#include <osmium/osm/node.hpp>
#include <osmium/osm/ostream.hpp>

BOOST_AUTO_TEST_SUITE(Buffer_Node)

void check_node_1(osmium::Node& node) {
    BOOST_CHECK_EQUAL(1, node.id());
    BOOST_CHECK_EQUAL(3, node.version());
    BOOST_CHECK_EQUAL(true, node.visible());
    BOOST_CHECK_EQUAL(333, node.changeset());
    BOOST_CHECK_EQUAL(21, node.uid());
    BOOST_CHECK_EQUAL(123, node.timestamp());
    BOOST_CHECK_EQUAL(osmium::Location(3.5, 4.7), node.location());
    BOOST_CHECK_EQUAL("testuser", node.user());

    for (osmium::memory::Item& item : node) {
        BOOST_CHECK_EQUAL(osmium::item_type::tag_list, item.type());
    }

    BOOST_CHECK_EQUAL(node.tags().begin(), node.tags().end());
    BOOST_CHECK(node.tags().empty());
    BOOST_CHECK_EQUAL(0, std::distance(node.tags().begin(), node.tags().end()));
}

void check_node_2(osmium::Node& node) {
    BOOST_CHECK_EQUAL(2, node.id());
    BOOST_CHECK_EQUAL(3, node.version());
    BOOST_CHECK_EQUAL(true, node.visible());
    BOOST_CHECK_EQUAL(333, node.changeset());
    BOOST_CHECK_EQUAL(21, node.uid());
    BOOST_CHECK_EQUAL(123, node.timestamp());
    BOOST_CHECK_EQUAL(osmium::Location(3.5, 4.7), node.location());
    BOOST_CHECK_EQUAL("testuser", node.user());

    for (osmium::memory::Item& item : node) {
        BOOST_CHECK_EQUAL(osmium::item_type::tag_list, item.type());
    }

    BOOST_CHECK(!node.tags().empty());
    BOOST_CHECK_EQUAL(2, std::distance(node.tags().begin(), node.tags().end()));

    int n = 0;
    for (osmium::Tag& tag : node.tags()) {
        switch (n) {
            case 0:
                BOOST_CHECK_EQUAL("amenity", tag.key());
                BOOST_CHECK_EQUAL("bank", tag.value());
                break;
            case 1:
                BOOST_CHECK_EQUAL("name", tag.key());
                BOOST_CHECK_EQUAL("OSM Savings", tag.value());
                break;
        }
        ++n;
    }
    BOOST_CHECK_EQUAL(2, n);
}

BOOST_AUTO_TEST_CASE(buffer_node) {
    constexpr size_t buffer_size = 10000;
    char data[buffer_size];

    osmium::memory::Buffer buffer(data, buffer_size, 0);

    {
        // add node 1
        osmium::memory::ObjectBuilder<osmium::Node> node_builder(buffer);
        osmium::Node& node = node_builder.object();
        BOOST_CHECK_EQUAL(osmium::item_type::node, node.type());

        node.id(1);
        node.version(3);
        node.visible(true);
        node.changeset(333);
        node.uid(21);
        node.timestamp(123);
        node.location(osmium::Location(3.5, 4.7));

        node_builder.add_string("testuser");

        buffer.commit();
    }

    {
        // add node 2
        osmium::memory::ObjectBuilder<osmium::Node> node_builder(buffer);
        osmium::Node& node = node_builder.object();
        BOOST_CHECK_EQUAL(osmium::item_type::node, node.type());

        node.id(2);
        node.version(3);
        node.visible(true);
        node.changeset(333);
        node.uid(21);
        node.timestamp(123);
        node.location(osmium::Location(3.5, 4.7));

        node_builder.add_string("testuser");

        {
            osmium::memory::TagListBuilder tag_builder(buffer, &node_builder);
            tag_builder.add_tag("amenity", "bank");
            tag_builder.add_tag("name", "OSM Savings");
            tag_builder.add_padding();
        }

        buffer.commit();
    }

    BOOST_CHECK_EQUAL(2, std::distance(buffer.begin(), buffer.end()));
    int item_no = 0;
    for (osmium::memory::Item& item : buffer) {
        BOOST_CHECK_EQUAL(osmium::item_type::node, item.type());

        osmium::Node& node = static_cast<osmium::Node&>(item);

        switch (item_no) {
            case 0:
                check_node_1(node);
                break;
            case 1:
                check_node_2(node);
                break;
            default:
                break;
        }

        ++item_no;

    }

}

BOOST_AUTO_TEST_SUITE_END()
