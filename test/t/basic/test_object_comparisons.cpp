#ifdef STAND_ALONE
# define BOOST_TEST_MODULE Main
#endif
#include <boost/test/unit_test.hpp>

#include <osmium/builder/osm_object_builder.hpp>
#include <osmium/osm/object_comparisons.hpp>
#include <osmium/osm.hpp>
#include <osmium/osm/ostream.hpp>

BOOST_AUTO_TEST_SUITE(Object_Comparisons)

BOOST_AUTO_TEST_CASE(order) {
    osmium::memory::Buffer buffer(10 * 1000);

    {
        // add node 1
        osmium::builder::NodeBuilder node_builder(buffer);
        node_builder.add_user("testuser");
        buffer.commit();
    }

    {
        // add node 2
        osmium::builder::NodeBuilder node_builder(buffer);
        node_builder.add_user("testuser");
        buffer.commit();
    }

    auto it = buffer.begin();
    osmium::Node& node1 = static_cast<osmium::Node&>(*it);
    osmium::Node& node2 = static_cast<osmium::Node&>(*(++it));

    node1.id(10);
    node1.version(1);
    node2.id(15);
    node2.version(2);
    BOOST_CHECK_EQUAL(true, node1 < node2);
    BOOST_CHECK_EQUAL(false, node1 > node2);
    node1.id(20);
    node1.version(1);
    node2.id(20);
    node2.version(2);
    BOOST_CHECK_EQUAL(true, node1 < node2);
    BOOST_CHECK_EQUAL(false, node1 > node2);
    node1.id(-10);
    node1.version(2);
    node2.id(-15);
    node2.version(1);
    BOOST_CHECK_EQUAL(true, node1 < node2);
    BOOST_CHECK_EQUAL(false, node1 > node2);
}

BOOST_AUTO_TEST_CASE(order_types) {
    osmium::memory::Buffer buffer(10 * 1000);

    {
        // add node 1
        osmium::builder::NodeBuilder node_builder(buffer);
        osmium::Node& node = node_builder.object();
        BOOST_CHECK_EQUAL(osmium::item_type::node, node.type());

        node.id(3);
        node.version(3);
        node_builder.add_user("testuser");

        buffer.commit();
    }

    {
        // add node 2
        osmium::builder::NodeBuilder node_builder(buffer);
        osmium::Node& node = node_builder.object();
        BOOST_CHECK_EQUAL(osmium::item_type::node, node.type());

        node.id(3);
        node.version(4);
        node_builder.add_user("testuser");

        buffer.commit();
    }

    {
        // add node 3
        osmium::builder::NodeBuilder node_builder(buffer);
        osmium::Node& node = node_builder.object();
        BOOST_CHECK_EQUAL(osmium::item_type::node, node.type());

        node.id(3);
        node.version(4);
        node_builder.add_user("testuser");

        buffer.commit();
    }

    {
        // add way
        osmium::builder::WayBuilder way_builder(buffer);
        osmium::Way& way = way_builder.object();
        BOOST_CHECK_EQUAL(osmium::item_type::way, way.type());

        way.id(2);
        way.version(2);
        way_builder.add_user("testuser");

        buffer.commit();
    }

    {
        // add relation
        osmium::builder::RelationBuilder relation_builder(buffer);
        osmium::Relation& relation = relation_builder.object();
        BOOST_CHECK_EQUAL(osmium::item_type::relation, relation.type());

        relation.id(1);
        relation.version(1);
        relation_builder.add_user("testuser");

        buffer.commit();
    }

    auto it = buffer.begin();
    const osmium::Node& node1 = static_cast<const osmium::Node&>(*it);
    const osmium::Node& node2 = static_cast<const osmium::Node&>(*(++it));
    const osmium::Node& node3 = static_cast<const osmium::Node&>(*(++it));
    const osmium::Way& way = static_cast<const osmium::Way&>(*(++it));
    const osmium::Relation& relation = static_cast<const osmium::Relation&>(*(++it));

    BOOST_CHECK_EQUAL(true, node1 < node2);
    BOOST_CHECK_EQUAL(true, node2 < way);
    BOOST_CHECK_EQUAL(false, node2 > way);
    BOOST_CHECK_EQUAL(true, way < relation);
    BOOST_CHECK_EQUAL(true, node1 < relation);

    BOOST_CHECK_EQUAL(true, osmium::object_order_type_id_version()(node1, node2));
    BOOST_CHECK_EQUAL(true, osmium::object_order_type_id_reverse_version()(node2, node1));
    BOOST_CHECK_EQUAL(true, osmium::object_order_type_id_version()(node1, way));
    BOOST_CHECK_EQUAL(true, osmium::object_order_type_id_reverse_version()(node1, way));

    BOOST_CHECK_EQUAL(false, osmium::object_equal_type_id_version()(node1, node2));
    BOOST_CHECK_EQUAL(true, osmium::object_equal_type_id_version()(node2, node3));

    BOOST_CHECK_EQUAL(true, osmium::object_equal_type_id()(node1, node2));
    BOOST_CHECK_EQUAL(true, osmium::object_equal_type_id()(node2, node3));

    BOOST_CHECK_EQUAL(false, osmium::object_equal_type_id_version()(node1, way));
    BOOST_CHECK_EQUAL(false, osmium::object_equal_type_id_version()(node1, relation));
    BOOST_CHECK_EQUAL(false, osmium::object_equal_type_id()(node1, relation));
}

BOOST_AUTO_TEST_SUITE_END()
