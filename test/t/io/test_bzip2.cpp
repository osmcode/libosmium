#ifdef STAND_ALONE
# define BOOST_TEST_MODULE Main
#endif
#include <boost/test/unit_test.hpp>
#include <boost/test/output_test_stream.hpp>
using boost::test_tools::output_test_stream;

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <osmium/io/bzip2_compression.hpp>

BOOST_AUTO_TEST_SUITE(Bzip2)

BOOST_AUTO_TEST_CASE(read_compressed_file) {
    int fd = ::open("t/io/data_bzip2.txt.bz2", O_RDONLY);
    BOOST_CHECK_GT(fd, 0);

    size_t size = 0;
    std::string all;
    {
        osmium::io::Bzip2Decompressor decomp(fd);
        for (std::string data = decomp.read(); !data.empty(); data = decomp.read()) {
            size += data.size();
            all += data;
        }
    }

    BOOST_CHECK_EQUAL(9, size);
    BOOST_CHECK_EQUAL("TESTDATA\n", all);
}

BOOST_AUTO_TEST_SUITE_END()

