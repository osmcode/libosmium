
namespace testcase_110 {

    class TestHandler : public osmium::handler::Handler {

    public:

        TestHandler() :
            osmium::handler::Handler() {
        }

        void node(const osmium::Node& node) {
            if (node.id() == 110000) {
                assert(node.location().lon() == 1.02);
                assert(node.location().lat() == 1.12);
            } else if (node.id() == 110001) {
                assert(node.location().lon() == 1.07);
                assert(node.location().lat() == 1.13);
            } else {
                throw std::runtime_error("Unknown ID");
            }
        }

        void way(const osmium::Way& way) {
            if (way.id() == 110800) {
                assert(way.version() == 1);
                assert(way.nodes().size() == 2);
                assert(!way.is_closed());

                const char *test_id = way.tags().get_value_by_key("test:id");
                assert(test_id && !strcmp(test_id, "110"));
            } else {
                throw std::runtime_error("Unknown ID");
            }
        }

    }; // class TestHandler

    struct RunTest {

        bool operator()(const std::string& dirname) {
            osmium::io::Reader reader(dirname + "/1-basic-geom/110/data.osm");

            index_pos_type index_pos;
            index_neg_type index_neg;
            location_handler_type location_handler(index_pos, index_neg);
            location_handler.ignore_errors();

            CheckBasicsHandler check_basics_handler(110, 2, 1, 0);
            CheckWKTHandler check_wkt_handler(dirname + "/1-basic-geom/110/out.wkt");
            TestHandler test_handler;

            osmium::apply(reader, location_handler, check_basics_handler, check_wkt_handler, test_handler);

            return true;
        }

    }; // RunTest

} // namespace testcase_110

