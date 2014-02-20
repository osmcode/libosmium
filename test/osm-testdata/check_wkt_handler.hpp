#ifndef CHECK_WKT_HANDLER_HPP
#define CHECK_WKT_HANDLER_HPP

#include <cassert>
#include <fstream>
#include <map>
#include <sstream>
#include <string>

#include <osmium/handler.hpp>
#include <osmium/osm.hpp>
#include <osmium/osm/types.hpp>

class CheckWKTHandler : public osmium::handler::Handler {

    std::map<osmium::object_id_type, std::string> m_geometries {};
    osmium::geom::WKTFactory m_factory {};

public:

    CheckWKTHandler(const std::string& filename) :
        osmium::handler::Handler() {
        std::ifstream ifs(filename, std::ifstream::in);

        if (!ifs) {
            std::cerr << "  Can't open out.wkt\n";
            exit(1);
        }

        osmium::object_id_type id;
        std::string line;
        while (std::getline(ifs, line)) {
            size_t pos = line.find_first_of(' ');
            assert(pos != std::string::npos && "out.wkt not formatted correctly");

            std::string id_str = line.substr(0, pos);
            std::istringstream iss(id_str);
            iss >> id;

            assert(m_geometries.find(id) == m_geometries.end() && "id contained twice in out.wkt");

            m_geometries[id] = line.substr(pos+1);
        }
    }

    void node(const osmium::Node& node) {
        const std::string wkt = m_geometries[node.id()];
        assert(wkt != "" && "Missing geometry for node in out.wkt");

        std::string this_wkt = m_factory.create_point(node.location());
        assert(wkt == this_wkt && "wkt geometries don't match");
        m_geometries.erase(node.id());
    }

    void way(const osmium::Way& way) {
        const std::string wkt = m_geometries[way.id()];
        assert(wkt != "" && "Missing geometry for way in out.wkt");

        std::string this_wkt = m_factory.create_linestring(way);
        assert(wkt == this_wkt && "wkt geometries don't match");
        m_geometries.erase(way.id());
    }

    void done() {
        assert(m_geometries.empty() && "out.wkt contains geometries not in data.osm");
    }

}; // CheckWKTHandler


#endif // CHECK_WKT_HANDLER_HPP
