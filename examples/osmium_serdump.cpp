/*

  The code in this example file is released into the Public Domain.

*/

#include <cerrno>
#include <cstring>
#include <iostream>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>

#ifdef _MSC_VER
# include <direct.h>
#endif

#include <osmium/io/any_input.hpp>
#include <osmium/handler/disk_store.hpp>
#include <osmium/handler/object_relations.hpp>

#include <osmium/index/map/sparse_mem_array.hpp>
#include <osmium/index/multimap/sparse_mem_multimap.hpp>
#include <osmium/index/multimap/sparse_mem_array.hpp>
#include <osmium/index/multimap/hybrid.hpp>

// ==============================================================================
// Choose the following depending on the size of the input OSM files:
// ==============================================================================
// for smaller OSM files (extracts)
using offset_index_type =
    osmium::index::map::SparseMemArray<osmium::unsigned_object_id_type, size_t>;
//  osmium::index::map::SparseMapMmap<osmium::unsigned_object_id_type, size_t>;
//  osmium::index::map::SparseMapFile<osmium::unsigned_object_id_type, size_t>;

using map_type =
    osmium::index::multimap::SparseMemArray<osmium::unsigned_object_id_type, osmium::unsigned_object_id_type>;
//  osmium::index::multimap::SparseMemMultimap<osmium::unsigned_object_id_type, osmium::unsigned_object_id_type>;
//  osmium::index::multimap::Hybrid<osmium::unsigned_object_id_type, osmium::unsigned_object_id_type>;

// ==============================================================================
// for very large OSM files (planet)
//using offset_index_type = osmium::index::map::DenseMmapArray<osmium::unsigned_object_id_type, size_t>;
// ==============================================================================


class IndexFile {

    int m_fd;

public:

    IndexFile(const std::string& filename) :
        m_fd(::open(filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666)) {
        if (m_fd < 0) {
            std::cerr << "Can't open index file '" << filename << "': " << std::strerror(errno) << "\n";
            std::exit(2);
        }
    }

    ~IndexFile() {
        if (m_fd >= 0) {
            close(m_fd);
        }
    }

    int fd() const noexcept {
        return m_fd;
    }

}; // class IndexFile

int main(int argc, char* argv[]) {
    std::ios_base::sync_with_stdio(false);

    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " OSMFILE DIR\n";
        std::exit(2);
    }

    const std::string input_file{argv[1]};
    const std::string dir{argv[2]};

#ifndef _WIN32
    const int result = ::mkdir(dir.c_str(), 0777);
#else
    const int result = mkdir(dir.c_str());
#endif
    if (result == -1 && errno != EEXIST) {
        std::cerr << "Problem creating directory '" << dir << "': " << std::strerror(errno) << "\n";
        std::exit(2);
    }

    const std::string data_file{dir + "/data.osm.ser"};
    const int data_fd = ::open(data_file.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (data_fd < 0) {
        std::cerr << "Can't open data file '" << data_file << "': " << std::strerror(errno) << "\n";
        std::exit(2);
    }

    offset_index_type node_index;
    offset_index_type way_index;
    offset_index_type relation_index;

    osmium::handler::DiskStore disk_store_handler{data_fd, node_index, way_index, relation_index};

    map_type map_node2way;
    map_type map_node2relation;
    map_type map_way2relation;
    map_type map_relation2relation;

    osmium::handler::ObjectRelations object_relations_handler(map_node2way, map_node2relation, map_way2relation, map_relation2relation);

    osmium::io::Reader reader{input_file};

    while (osmium::memory::Buffer buffer = reader.read()) {
        disk_store_handler(buffer); // XXX
        osmium::apply(buffer, object_relations_handler);
    }

    reader.close();

    IndexFile nodes_idx(dir + "/nodes.idx");
    node_index.dump_as_list(nodes_idx.fd());

    IndexFile ways_idx(dir + "/ways.idx");
    way_index.dump_as_list(ways_idx.fd());

    IndexFile relations_idx(dir + "/relations.idx");
    relation_index.dump_as_list(relations_idx.fd());

    map_node2way.sort();
    IndexFile node2way_idx(dir + "/node2way.map");
    map_node2way.dump_as_list(node2way_idx.fd());

    map_node2relation.sort();
    IndexFile node2relation_idx(dir + "/node2rel.map");
    map_node2relation.dump_as_list(node2relation_idx.fd());

    map_way2relation.sort();
    IndexFile way2relation_idx(dir + "/way2rel.map");
    map_way2relation.dump_as_list(way2relation_idx.fd());

    map_relation2relation.sort();
    IndexFile relation2relation_idx(dir + "/rel2rel.map");
    map_relation2relation.dump_as_list(relation2relation_idx.fd());
}

