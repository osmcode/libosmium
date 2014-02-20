#ifndef COUNT_OBJECTS_HANDLER_HPP
#define COUNT_OBJECTS_HANDLER_HPP

#include <cassert>

#include <osmium/handler.hpp>
#include <osmium/osm.hpp>

class CountObjectsHandler : public osmium::handler::Handler {

    int num_nodes;
    int num_ways;
    int num_relations;

public:

    CountObjectsHandler(int n, int w, int r) :
        osmium::handler::Handler(),
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


#endif // COUNT_OBJECTS_HANDLER_HPP
