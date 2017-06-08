#include "catch.hpp"
#include "utils.hpp"

#include <osmium/io/xml_input.hpp>
#include <osmium/relations/generic_relations_manager.hpp>

class TestAssembler {

public:

    void operator()(const osmium::Relation& relation, const std::vector<const osmium::OSMObject*>& members, osmium::memory::Buffer& buffer) {
        REQUIRE(relation.members().size() == members.size());
        switch (relation.id()) {
            case 30:
                REQUIRE(members.size() == 1);
                buffer.add_item(*members[0]);
                buffer.commit();
                break;
            case 31:
                REQUIRE(members.size() == 3);
                break;
            case 32:
                REQUIRE(members.size() == 1);
                break;
            default:
                REQUIRE(false);
                break;
        }
    }

}; // class TestAssembler


TEST_CASE("Generic relations manager") {
    osmium::io::File file{with_data_dir("t/relations/data.osm")};
    osmium::relations::GenericRelationsManager<TestAssembler> manager;

    osmium::relations::read_relations(file, manager);

    osmium::io::Reader reader{file};
    osmium::apply(reader, manager.handler());
    reader.close();

    auto buffer = manager.read();
    REQUIRE(std::distance(buffer.begin(), buffer.end()) == 1);

    const auto& obj = buffer.get<osmium::OSMObject>(0);
    REQUIRE(obj.type() == osmium::item_type::node);
    REQUIRE(obj.id() == 10);
}

