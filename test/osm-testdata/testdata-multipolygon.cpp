
#include <iostream>
#include <fstream>
#include <getopt.h>
#include <map>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wredundant-decls"
# include <ogr_api.h>
# include <ogrsf_frmts.h>
#pragma GCC diagnostic pop

#include <osmium/area/assembler.hpp>
#include <osmium/area/collector.hpp>
#include <osmium/geom/ogr.hpp>
#include <osmium/geom/wkt.hpp>
#include <osmium/handler.hpp>
#include <osmium/handler/node_locations_for_ways.hpp>
#include <osmium/index/map/dummy.hpp>
#include <osmium/index/map/stl_vector.hpp>
#include <osmium/io/xml_input.hpp>
#include <osmium/visitor.hpp>

typedef osmium::index::map::SparseMapMem<osmium::unsigned_object_id_type, osmium::Location> index_type;

typedef osmium::handler::NodeLocationsForWays<index_type, index_type> location_handler_type;

struct less_charptr {

    bool operator()(const char* a, const char* b) const {
        return std::strcmp(a, b) < 0;
    }

}; // less_charptr

typedef std::map<const char*, const char*, less_charptr> tagmap_type;

inline tagmap_type create_map(const osmium::TagList& taglist) {
    tagmap_type map;

    for (auto& tag : taglist) {
        map[tag.key()] = tag.value();
    }

    return map;
}

class TestHandler : public osmium::handler::Handler {

    OGRDataSource* m_data_source;
    OGRLayer* m_layer_point;
    OGRLayer* m_layer_linestring;
    OGRLayer* m_layer_polygon;
    OGRLayer* m_layer_perror;
    OGRLayer* m_layer_lerror;

    osmium::geom::OGRFactory m_ogr_factory {};
    osmium::geom::WKTFactory m_wkt_factory {};

    std::ofstream m_out;

    bool m_first_out{true};

public:

    TestHandler(const std::string& driver_name, const std::string& filename) :
        m_out("multipolygon-tests.json") {

        OGRRegisterAll();

        OGRSFDriver* driver = OGRSFDriverRegistrar::GetRegistrar()->GetDriverByName(driver_name.c_str());
        if (!driver) {
            std::cerr << driver_name << " driver not available.\n";
            exit(1);
        }

        CPLSetConfigOption("OGR_SQLITE_SYNCHRONOUS", "FALSE");
        const char* options[] = { "SPATIALITE=TRUE", nullptr };
        m_data_source = driver->CreateDataSource(filename.c_str(), const_cast<char**>(options));
        if (!m_data_source) {
            std::cerr << "Creation of output file failed.\n";
            exit(1);
        }

        OGRSpatialReference sparef;
        sparef.SetWellKnownGeogCS("WGS84");

        /**************/

        m_layer_point = m_data_source->CreateLayer("points", &sparef, wkbPoint, nullptr);
        if (!m_layer_point) {
            std::cerr << "Layer creation failed.\n";
            exit(1);
        }

        OGRFieldDefn layer_point_field_id("id", OFTReal);
        layer_point_field_id.SetWidth(10);

        if (m_layer_point->CreateField(&layer_point_field_id) != OGRERR_NONE) {
            std::cerr << "Creating id field failed.\n";
            exit(1);
        }

        OGRFieldDefn layer_point_field_type("type", OFTString);
        layer_point_field_type.SetWidth(30);

        if (m_layer_point->CreateField(&layer_point_field_type) != OGRERR_NONE) {
            std::cerr << "Creating type field failed.\n";
            exit(1);
        }

        /**************/

        m_layer_linestring = m_data_source->CreateLayer("lines", &sparef, wkbLineString, nullptr);
        if (!m_layer_linestring) {
            std::cerr << "Layer creation failed.\n";
            exit(1);
        }

        OGRFieldDefn layer_linestring_field_id("id", OFTReal);
        layer_linestring_field_id.SetWidth(10);

        if (m_layer_linestring->CreateField(&layer_linestring_field_id) != OGRERR_NONE) {
            std::cerr << "Creating id field failed.\n";
            exit(1);
        }

        OGRFieldDefn layer_linestring_field_type("type", OFTString);
        layer_linestring_field_type.SetWidth(30);

        if (m_layer_linestring->CreateField(&layer_linestring_field_type) != OGRERR_NONE) {
            std::cerr << "Creating type field failed.\n";
            exit(1);
        }

        /**************/

        m_layer_polygon = m_data_source->CreateLayer("multipolygons", &sparef, wkbMultiPolygon, nullptr);
        if (!m_layer_polygon) {
            std::cerr << "Layer creation failed.\n";
            exit(1);
        }

        OGRFieldDefn layer_polygon_field_id("id", OFTInteger);
        layer_polygon_field_id.SetWidth(10);

        if (m_layer_polygon->CreateField(&layer_polygon_field_id) != OGRERR_NONE) {
            std::cerr << "Creating id field failed.\n";
            exit(1);
        }

        OGRFieldDefn layer_polygon_field_from_type("from_type", OFTString);
        layer_polygon_field_from_type.SetWidth(1);

        if (m_layer_polygon->CreateField(&layer_polygon_field_from_type) != OGRERR_NONE) {
            std::cerr << "Creating from_type field failed.\n";
            exit(1);
        }

        /**************/

        m_layer_perror = m_data_source->CreateLayer("perrors", &sparef, wkbPoint, nullptr);
        if (!m_layer_perror) {
            std::cerr << "Layer creation failed.\n";
            exit(1);
        }

        OGRFieldDefn layer_perror_field_id1("id1", OFTReal);
        layer_perror_field_id1.SetWidth(10);

        if (m_layer_perror->CreateField(&layer_perror_field_id1) != OGRERR_NONE) {
            std::cerr << "Creating id1 field failed.\n";
            exit(1);
        }

        OGRFieldDefn layer_perror_field_id2("id2", OFTReal);
        layer_perror_field_id2.SetWidth(10);

        if (m_layer_perror->CreateField(&layer_perror_field_id2) != OGRERR_NONE) {
            std::cerr << "Creating id2 field failed.\n";
            exit(1);
        }

        OGRFieldDefn layer_perror_field_problem_type("problem_type", OFTString);
        layer_perror_field_problem_type.SetWidth(30);

        if (m_layer_perror->CreateField(&layer_perror_field_problem_type) != OGRERR_NONE) {
            std::cerr << "Creating problem_type field failed.\n";
            exit(1);
        }

        /**************/

        m_layer_lerror = m_data_source->CreateLayer("lerrors", &sparef, wkbLineString, nullptr);
        if (!m_layer_lerror) {
            std::cerr << "Layer creation failed.\n";
            exit(1);
        }

        OGRFieldDefn layer_lerror_field_id1("id1", OFTReal);
        layer_lerror_field_id1.SetWidth(10);

        if (m_layer_lerror->CreateField(&layer_lerror_field_id1) != OGRERR_NONE) {
            std::cerr << "Creating id1 field failed.\n";
            exit(1);
        }

        OGRFieldDefn layer_lerror_field_id2("id2", OFTReal);
        layer_lerror_field_id2.SetWidth(10);

        if (m_layer_lerror->CreateField(&layer_lerror_field_id2) != OGRERR_NONE) {
            std::cerr << "Creating id2 field failed.\n";
            exit(1);
        }

        OGRFieldDefn layer_lerror_field_problem_type("problem_type", OFTString);
        layer_lerror_field_problem_type.SetWidth(30);

        if (m_layer_lerror->CreateField(&layer_lerror_field_problem_type) != OGRERR_NONE) {
            std::cerr << "Creating problem_type field failed.\n";
            exit(1);
        }
    }

    ~TestHandler() {
        m_out << "\n]\n";
        OGRDataSource::DestroyDataSource(m_data_source);
        OGRCleanupAll();
    }

    void node(const osmium::Node& node) {
        OGRFeature* feature = OGRFeature::CreateFeature(m_layer_point->GetLayerDefn());
        std::unique_ptr<OGRPoint> ogr_point = m_ogr_factory.create_point(node);
        feature->SetGeometry(ogr_point.get());
        feature->SetField("id", static_cast<double>(node.id()));
        feature->SetField("type", node.tags().get_value_by_key("type"));

        if (m_layer_point->CreateFeature(feature) != OGRERR_NONE) {
            std::cerr << "Failed to create feature.\n";
            exit(1);
        }

        OGRFeature::DestroyFeature(feature);
    }

    void way(const osmium::Way& way) {
        try {
            std::unique_ptr<OGRLineString> ogr_linestring = m_ogr_factory.create_linestring(way);
            OGRFeature* feature = OGRFeature::CreateFeature(m_layer_linestring->GetLayerDefn());
            feature->SetGeometry(ogr_linestring.get());
            feature->SetField("id", static_cast<double>(way.id()));
            feature->SetField("type", way.tags().get_value_by_key("type"));

            if (m_layer_linestring->CreateFeature(feature) != OGRERR_NONE) {
                std::cerr << "Failed to create feature.\n";
                exit(1);
            }

            OGRFeature::DestroyFeature(feature);
        } catch (osmium::geom::geometry_error&) {
            std::cerr << "Ignoring illegal geometry for way " << way.id() << ".\n";
        }
    }

    void area(const osmium::Area& area) {
        if (m_first_out) {
            m_out << "[\n";
            m_first_out = false;
        } else {
            m_out << ",\n";
        }
        m_out << "{\n  \"test_id\": " << (area.orig_id() / 1000) << ",\n  \"area_id\": " << area.id() << ",\n  \"from_id\": " << area.orig_id() << ",\n  \"from_type\": \"" << (area.from_way() ? "way" : "relation") << "\",\n  \"wkt\": \"";
        try {
            std::string wkt = m_wkt_factory.create_multipolygon(area);
            m_out << wkt << "\",\n  \"tags\": {";

            auto tagmap = create_map(area.tags());
            bool first = true;
            for (auto& tag : tagmap) {
                if (first) {
                    first = false;
                } else {
                    m_out << ", ";
                }
                m_out << '"' << tag.first << "\": \"" << tag.second << '"';
            }
            m_out << "}\n}";
        } catch (osmium::geom::geometry_error&) {
            m_out << "INVALID\"\n}";
        }
        try {
            std::unique_ptr<OGRMultiPolygon> ogr_polygon = m_ogr_factory.create_multipolygon(area);
            OGRFeature* feature = OGRFeature::CreateFeature(m_layer_polygon->GetLayerDefn());
            feature->SetGeometry(ogr_polygon.get());
            feature->SetField("id", static_cast<int>(area.orig_id()));

            std::string from_type;
            if (area.from_way()) {
                from_type = "w";
            } else {
                from_type = "r";
            }
            feature->SetField("from_type", from_type.c_str());

            if (m_layer_polygon->CreateFeature(feature) != OGRERR_NONE) {
                std::cerr << "Failed to create feature.\n";
                exit(1);
            }

            OGRFeature::DestroyFeature(feature);
        } catch (osmium::geom::geometry_error&) {
            std::cerr << "Ignoring illegal geometry for area " << area.id() << " created from " << (area.from_way() ? "way" : "relation") << " with id=" << area.orig_id() << ".\n";
        }
    }

    void write_problem_points(const std::vector<osmium::area::ProblemPoint>& problems) {
        for (auto& problem : problems) {
            OGRFeature* feature = OGRFeature::CreateFeature(m_layer_perror->GetLayerDefn());
            std::unique_ptr<OGRPoint> ogr_point = m_ogr_factory.create_point(problem.location());
            feature->SetGeometry(ogr_point.get());
            feature->SetField("id1", static_cast<double>(problem.id1()));
            feature->SetField("id2", static_cast<double>(problem.id2()));
            feature->SetField("problem_type", problem.type_string().c_str());

            if (m_layer_perror->CreateFeature(feature) != OGRERR_NONE) {
                std::cerr << "Failed to create feature.\n";
                exit(1);
            }

            OGRFeature::DestroyFeature(feature);
        }
    }

    void write_problem_lines(const std::vector<osmium::area::ProblemLine>& problems) {
        for (auto& problem : problems) {
            std::unique_ptr<OGRPoint> ogr_point1 = m_ogr_factory.create_point(problem.segment().first());
            std::unique_ptr<OGRPoint> ogr_point2 = m_ogr_factory.create_point(problem.segment().second());
            std::unique_ptr<OGRLineString> ogr_linestring = std::unique_ptr<OGRLineString>(new OGRLineString());
            ogr_linestring->addPoint(ogr_point1.get());
            ogr_linestring->addPoint(ogr_point2.get());
            OGRFeature* feature = OGRFeature::CreateFeature(m_layer_lerror->GetLayerDefn());
            feature->SetGeometry(ogr_linestring.get());
            feature->SetField("id1", static_cast<double>(problem.id1()));
            feature->SetField("id2", static_cast<double>(problem.id2()));
            feature->SetField("problem_type", problem.type_string().c_str());

            if (m_layer_lerror->CreateFeature(feature) != OGRERR_NONE) {
                std::cerr << "Failed to create feature.\n";
                exit(1);
            }

            OGRFeature::DestroyFeature(feature);
        }
    }

};

/* ================================================== */

void print_help() {
    std::cout << "testdata-multipolygon [OPTIONS] [INFILE [OUTFILE]]\n\n" \
              << "If INFILE is not given stdin is assumed.\n" \
              << "If OUTFILE is not given 'multipolygon.db' is used.\n" \
              << "\nOptions:\n" \
              << "  -h, --help           This help message\n" \
              << "  -f, --format=FORMAT  Output OGR format (Default: 'SQLite')\n";
}

int main(int argc, char* argv[]) {
    static struct option long_options[] = {
        {"help",   no_argument, 0, 'h'},
        {"format", required_argument, 0, 'f'},
        {0, 0, 0, 0}
    };

    std::string output_format("SQLite");

    while (true) {
        int c = getopt_long(argc, argv, "hf:", long_options, 0);
        if (c == -1) {
            break;
        }

        switch (c) {
            case 'h':
                print_help();
                exit(0);
            case 'f':
                output_format = optarg;
                break;
            default:
                exit(1);
        }
    }

    std::string input_filename;
    std::string output_filename("multipolygon.db");
    int remaining_args = argc - optind;
    if (remaining_args > 2) {
        std::cerr << "Usage: " << argv[0] << " [OPTIONS] [INFILE [OUTFILE]]" << std::endl;
        exit(1);
    } else if (remaining_args == 2) {
        input_filename =  argv[optind];
        output_filename = argv[optind+1];
    } else if (remaining_args == 1) {
        input_filename =  argv[optind];
    } else {
        input_filename = "-";
    }

    typedef osmium::area::Assembler area_assembler_type;
    area_assembler_type assembler;
    assembler.enable_debug_output();
    assembler.remember_problems();
    osmium::area::Collector<area_assembler_type> collector(assembler);

    std::cerr << "Pass 1...\n";
    osmium::io::Reader reader1(input_filename);
    collector.read_relations(reader1);
    std::cerr << "Pass 1 done\n";

    index_type index_pos;
    index_type index_neg;
    location_handler_type location_handler(index_pos, index_neg);
    location_handler.ignore_errors();

    TestHandler ogr_handler(output_format, output_filename);

    std::cerr << "Pass 2...\n";
    osmium::io::Reader reader2(input_filename);
    osmium::apply(reader2, location_handler, ogr_handler, collector.handler());
    reader2.close();
    std::cerr << "Pass 2 done\n";

    osmium::apply(collector, ogr_handler);

    ogr_handler.write_problem_points(assembler.problem_points());
    ogr_handler.write_problem_lines(assembler.problem_lines());
}

