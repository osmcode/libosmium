/* The code in this file is released into the Public Domain. */

#include <cassert>
#include <iostream>
#include <string>

#include <osmium/geom/wkt.hpp>
#include <osmium/handler.hpp>
#include <osmium/handler/node_locations_for_ways.hpp>
#include <osmium/index/map/dummy.hpp>
#include <osmium/index/map/stl_vector.hpp>
#include <osmium/io/any_input.hpp>
#include <osmium/visitor.hpp>

typedef osmium::index::map::Dummy<osmium::unsigned_object_id_type, osmium::Location> index_neg_type;
typedef osmium::index::map::SparseMapMem<osmium::unsigned_object_id_type, osmium::Location> index_pos_type;
typedef osmium::handler::NodeLocationsForWays<index_pos_type, index_neg_type> location_handler_type;

/* ================================================== */

class CountObjectsHandler : public osmium::handler::Handler {

    int num_nodes;
    int num_ways;
    int num_relations;

public:

    CountObjectsHandler(int n, int w, int r) :
        num_nodes(n),
        num_ways(w),
        num_relations(r) {
    }

    void node(const osmium::Node&) {
        --num_nodes;
    }

    void way(const osmium::Way&) {
        --num_ways;
    }

    void relations(const osmium::Relation&) {
        --num_relations;
    }

    void done() {
        assert(num_nodes == 0 && "Wrong number of nodes in data.osm");
        assert(num_ways == 0 && "Wrong number of ways in data.osm");
        assert(num_relations == 0 && "Wrong number of relations in data.osm");
    }

}; // CountObjectsHandler

/* ================================================== */

#include "tests-include.hpp"

/* ================================================== */

int main(int argc, char* argv[]) {

    if (argc != 2) {
        std::cerr << "Usage: testdata-read TEST-DIRECTORY\n";
        exit(3);
    }

    std::string directory = argv[1];

#include "tests-run.hpp"

    google::protobuf::ShutdownProtobufLibrary();
}

