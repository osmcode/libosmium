#ifdef STAND_ALONE
# define BOOST_TEST_MODULE Main
#endif
#include <boost/test/unit_test.hpp>
#include <boost/test/output_test_stream.hpp>
using boost::test_tools::output_test_stream;

#include <osmium/index/detail/typed_mmap.hpp>

BOOST_AUTO_TEST_SUITE(TypedMmap)

BOOST_AUTO_TEST_CASE(Mmap) {
    uint64_t* data = osmium::detail::typed_mmap<uint64_t>::map(10);

    data[0] = 4;
    data[3] = 9;
    data[9] = 25;

    BOOST_CHECK_EQUAL(4, data[0]);
    BOOST_CHECK_EQUAL(9, data[3]);
    BOOST_CHECK_EQUAL(25, data[9]);

    osmium::detail::typed_mmap<uint64_t>::unmap(data, 10);
}

BOOST_AUTO_TEST_CASE(MmapSizeZero) {
    BOOST_CHECK_THROW(osmium::detail::typed_mmap<uint64_t>::map(0), std::system_error);
}

BOOST_AUTO_TEST_CASE(MmapHugeSize) {
    BOOST_CHECK_THROW(osmium::detail::typed_mmap<uint64_t>::map(1L << 50), std::system_error);
}

#ifdef __linux__
BOOST_AUTO_TEST_CASE(Remap) {
    uint64_t* data = osmium::detail::typed_mmap<uint64_t>::map(10);

    data[0] = 4;
    data[3] = 9;
    data[9] = 25;

    uint64_t* new_data = osmium::detail::typed_mmap<uint64_t>::remap(data, 10, 1000);

    BOOST_CHECK_EQUAL(4, new_data[0]);
    BOOST_CHECK_EQUAL(9, new_data[3]);
    BOOST_CHECK_EQUAL(25, new_data[9]);
}
#else
# pragma message "not running 'Remap' test case on this machine"
#endif

BOOST_AUTO_TEST_CASE(FileSize) {
    const int size = 100;
    char filename[] = "/tmp/osmium_unit_test_XXXXXX";
    const int fd = mkstemp(filename);
    BOOST_CHECK_GT(fd, 0);
    BOOST_CHECK_EQUAL(0, unlink(filename));
    BOOST_CHECK_EQUAL(0, osmium::detail::typed_mmap<uint64_t>::file_size(fd));
    BOOST_CHECK_EQUAL(0, ftruncate(fd, size * sizeof(uint64_t)));
    BOOST_CHECK_EQUAL(size, osmium::detail::typed_mmap<uint64_t>::file_size(fd));

    osmium::detail::typed_mmap<uint64_t>::grow_file(size / 2, fd);
    BOOST_CHECK_EQUAL(size, osmium::detail::typed_mmap<uint64_t>::file_size(fd));

    osmium::detail::typed_mmap<uint64_t>::grow_file(size, fd);
    BOOST_CHECK_EQUAL(size, osmium::detail::typed_mmap<uint64_t>::file_size(fd));

    osmium::detail::typed_mmap<uint64_t>::grow_file(size * 2, fd);
    BOOST_CHECK_EQUAL(size * 2, osmium::detail::typed_mmap<uint64_t>::file_size(fd));
}

BOOST_AUTO_TEST_CASE(GrowAndMap) {
    const int size = 100;
    char filename[] = "/tmp/osmium_unit_test_XXXXXX";
    const int fd = mkstemp(filename);
    BOOST_CHECK_GT(fd, 0);
    BOOST_CHECK_EQUAL(0, unlink(filename));

    uint64_t* data = osmium::detail::typed_mmap<uint64_t>::grow_and_map(size, fd);
    BOOST_CHECK_EQUAL(size, osmium::detail::typed_mmap<uint64_t>::file_size(fd));

    data[0] = 1;
    data[1] = 8;
    data[99] = 27;

    BOOST_CHECK_EQUAL(1, data[0]);
    BOOST_CHECK_EQUAL(8, data[1]);
    BOOST_CHECK_EQUAL(27, data[99]);

    osmium::detail::typed_mmap<uint64_t>::unmap(data, size);
}

BOOST_AUTO_TEST_SUITE_END()
