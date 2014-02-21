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

#include "check_basics_handler.hpp"
#include "check_wkt_handler.hpp"

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

