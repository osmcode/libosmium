/*

  This is an example tool that converts OSM data to some output format
  like Spatialite or Shapefiles using the OGR library.

  The code in this example file is released into the Public Domain.

*/

#include <iostream>
#include <getopt.h>

#include <ogr_api.h>
#include <ogrsf_frmts.h>

// usually you only need one or two of these
#include <osmium/index/map/dummy.hpp>
#include <osmium/index/map/fixed_array.hpp>
#include <osmium/index/map/mmap_anon.hpp>
#include <osmium/index/map/mmap_file.hpp>
#include <osmium/index/map/sparse_table.hpp>
#include <osmium/index/map/std_map.hpp>
#include <osmium/index/map/vector.hpp>

#include <osmium/handler/coordinates_for_ways.hpp>

#include <osmium/geom/ogr.hpp>
#include <osmium/io/any_input.hpp>

typedef osmium::index::map::Dummy<osmium::object_id_type, osmium::Location> index_neg_type;
typedef osmium::index::map::SparseTable<osmium::object_id_type, osmium::Location> index_pos_type;

typedef osmium::handler::CoordinatesForWays<index_pos_type, index_neg_type> cfw_handler_type;

class MyOGRHandler : public osmium::handler::Handler<MyOGRHandler> {

    OGRDataSource* m_data_source;
    OGRLayer* m_layer_point;
    OGRLayer* m_layer_linestring;

public:

    MyOGRHandler(const std::string& driver_name, const std::string& filename) {

        OGRRegisterAll();

        OGRSFDriver* driver = OGRSFDriverRegistrar::GetRegistrar()->GetDriverByName(driver_name.c_str());
        if (driver == NULL) {
            std::cerr << driver_name << " driver not available.\n";
            exit(1);
        }

        CPLSetConfigOption("OGR_SQLITE_SYNCHRONOUS", "FALSE");
        const char* options[] = { "SPATIALITE=TRUE", NULL };
        m_data_source = driver->CreateDataSource(filename.c_str(), const_cast<char**>(options));
        if (m_data_source == NULL) {
            std::cerr << "Creation of output file failed.\n";
            exit(1);
        }

        OGRSpatialReference sparef;
        sparef.SetWellKnownGeogCS("WGS84");
        m_layer_point = m_data_source->CreateLayer("postboxes", &sparef, wkbPoint, NULL);
        if (m_layer_point == NULL) {
            std::cerr << "Layer creation failed.\n";
            exit(1);
        }

        OGRFieldDefn layer_point_field_id("id", OFTReal);
        layer_point_field_id.SetWidth(10);

        if (m_layer_point->CreateField(&layer_point_field_id) != OGRERR_NONE) {
            std::cerr << "Creating id field failed.\n";
            exit(1);
        }

        OGRFieldDefn layer_point_field_operator("operator", OFTString);
        layer_point_field_operator.SetWidth(30);

        if (m_layer_point->CreateField(&layer_point_field_operator) != OGRERR_NONE) {
            std::cerr << "Creating operator field failed.\n";
            exit(1);
        }

        /* Transactions might make things faster, then again they might not.
           Feel free to experiment and benchmark and report back. */
        m_layer_point->StartTransaction();

        m_layer_linestring = m_data_source->CreateLayer("roads", &sparef, wkbLineString, NULL);
        if (m_layer_linestring == NULL) {
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

        m_layer_linestring->StartTransaction();
    }

    ~MyOGRHandler() {
        OGRDataSource::DestroyDataSource(m_data_source);
        OGRCleanupAll();
    }

    void node(const osmium::Node& node) {
        const char* amenity = node.tags().get_value_by_key("amenity");
        if (amenity && !strcmp(amenity, "post_box")) {
            OGRFeature* feature = OGRFeature::CreateFeature(m_layer_point->GetLayerDefn());
            std::unique_ptr<OGRPoint> ogr_point = osmium::geom::create_ogr_point(node);
            feature->SetGeometry(ogr_point.get());
            feature->SetField("id", static_cast<double>(node.id()));
            feature->SetField("operator", node.tags().get_value_by_key("operator"));

            if (m_layer_point->CreateFeature(feature) != OGRERR_NONE) {
                std::cerr << "Failed to create feature.\n";
                exit(1);
            }

            OGRFeature::DestroyFeature(feature);
        }
    }

    void after_nodes() {
        m_layer_point->CommitTransaction();
    }

    void way(const osmium::Way& way) {
        const char* highway = way.tags().get_value_by_key("highway");
        if (highway) {
//            try {
//                Osmium::Geometry::LineString linestring(*way);

                OGRFeature* feature = OGRFeature::CreateFeature(m_layer_linestring->GetLayerDefn());
                std::unique_ptr<OGRLineString> ogr_linestring = osmium::geom::create_ogr_linestring(way.nodes());
                feature->SetGeometry(ogr_linestring.get());
                feature->SetField("id", static_cast<double>(way.id()));
                feature->SetField("type", highway);

                if (m_layer_linestring->CreateFeature(feature) != OGRERR_NONE) {
                    std::cerr << "Failed to create feature.\n";
                    exit(1);
                }

                OGRFeature::DestroyFeature(feature);
/*            } catch (Osmium::Geometry::IllegalGeometry) {
                std::cerr << "Ignoring illegal geometry for way " << way->id() << ".\n";
            }*/
        }
    }

    void after_ways() {
        m_layer_linestring->CommitTransaction();
    }

    void init() const {
        std::cerr << "init\n";
    }

    void before_nodes() const {
        std::cerr << "before_nodes\n";
    }

    void after_nodes() const {
        std::cerr << "after_nodes\n";
    }

    void before_ways() const {
        std::cerr << "before_ways\n";
    }

    void after_ways() const {
        std::cerr << "after_ways\n";
    }

    void before_relations() const {
        std::cerr << "before_relations\n";
    }

    void after_relations() const {
        std::cerr << "after_relations\n";
    }

    void done() const {
        std::cerr << "done\n";
    }

};

/* ================================================== */

void print_help() {
    std::cout << "osmium_toogr [OPTIONS] [INFILE [OUTFILE]]\n\n" \
              << "If INFILE is not given stdin is assumed.\n" \
              << "If OUTFILE is not given 'ogr_out' is used.\n" \
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
    std::string output_filename("ogr_out");
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

    osmium::io::Reader reader(input_filename);
    osmium::io::Meta meta = reader.open();

    index_pos_type index_pos;
    index_neg_type index_neg;
    cfw_handler_type cfw(index_pos, index_neg);

    MyOGRHandler ogr(output_format, output_filename);

    reader.push(cfw, ogr);
/*    while (osmium::memory::Buffer buffer = reader.read()) {
        cfw(buffer);
        ogr(buffer);
        delete[] buffer.data();
    }*/

    google::protobuf::ShutdownProtobufLibrary();
}

