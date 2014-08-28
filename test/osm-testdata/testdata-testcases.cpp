/* The code in this file is released into the Public Domain. */

#include <cassert>
#include <iostream>
#include <string>

#define CATCH_CONFIG_RUNNER
#include <catch.hpp>

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

static std::string dirname;

#include "tests-include.hpp"

/* ================================================== */

int main(int argc, char* argv[]) {
    const char* testcases_dir = getenv("TESTCASES_DIR");
    if (testcases_dir) {
        dirname = testcases_dir;
        std::cerr << "Running tests from '" << dirname << "' (from TESTCASES_DIR environment variable)\n";
    } else {
        std::cerr << "Please set TESTCASES_DIR environment variable.\n";
        exit(1);
    }

    int result = Catch::Session().run(argc, argv);

    google::protobuf::ShutdownProtobufLibrary();

    return result;
}

