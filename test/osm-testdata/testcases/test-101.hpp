
namespace testcase_101 {

    class TestHandler : public osmium::handler::Handler {

    public:

        TestHandler() :
            osmium::handler::Handler() {
        }

        void node(osmium::Node& node) {
            if (node.id() == 101000) {
                assert(node.version() == 1);
                assert(node.location().lon() == 1.12);
                assert(node.location().lat() == 1.02);
            } else if (node.id() == 101001) {
                assert(node.version() == 1);
                assert(node.location().lon() == 1.12);
                assert(node.location().lat() == 1.03);
            } else if (node.id() == 101002) {
            } else if (node.id() == 101003) {
            } else {
                throw std::runtime_error("Unknown ID");
            }
        }

    }; // class TestHandler

    struct RunTest {

        bool operator()(const std::string& dirname) {
            osmium::io::Reader reader(dirname + "/1-basic-geom/101/data.osm");

            CheckBasicsHandler check_basics_handler(101, 4, 0, 0);
            CheckWKTHandler check_wkt_handler(dirname, "1-basic-geom", 101);
            TestHandler test_handler;

            osmium::apply(reader, check_basics_handler, test_handler);

            return true;
        }

    }; // RunTest

} // namespace testcase_101

