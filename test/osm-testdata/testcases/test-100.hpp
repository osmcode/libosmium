
namespace testcase_100 {

    class TestHandler : public osmium::handler::Handler {

    public:

        TestHandler() :
            osmium::handler::Handler() {
        }

        void node(osmium::Node& node) {
            if (node.id() == 100000) {
                assert(node.version() == 1);
                assert(node.timestamp() == osmium::Timestamp("2014-01-01T00:00:00Z"));
                assert(node.uid() == 1);
                assert(!strcmp(node.user(), "test"));
                assert(node.changeset() == 1);
                assert(node.location().lon() == 1.02);
                assert(node.location().lat() == 1.02);
            } else {
                throw std::runtime_error("Unknown ID");
            }
        }

    }; // class TestHandler

    struct RunTest {

        bool operator()(const std::string& dirname) {
            osmium::io::Reader reader(dirname + "/1-basic-geom/100/data.osm");

            CountObjectsHandler count_handler(1, 0, 0);
            CheckWKTHandler check_wkt_handler(dirname + "/1-basic-geom/100/out.wkt");
            TestHandler test_handler;

            osmium::apply(reader, count_handler, check_wkt_handler, test_handler);

            return true;
        }

    }; // RunTest

} // namespace testcase_100

