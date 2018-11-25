#include "catch.hpp"

#include "utils.hpp"

#include <osmium/io/detail/read_write.hpp>
#include <osmium/io/gzip_compression.hpp>

#include <string>

TEST_CASE("Invalid file descriptor of gzip-compressed file") {
    REQUIRE_THROWS_AS(osmium::io::GzipDecompressor{-1}, const std::system_error&);
}

TEST_CASE("Non-open file descriptor of gzip-compressed file") {
    // 12345 is just a random file descriptor that should not be open
    REQUIRE_THROWS_AS(osmium::io::GzipDecompressor{12345}, const std::system_error&);
}

TEST_CASE("Empty gzip-compressed file") {
    const int count1 = count_fds();

    const std::string input_file = with_data_dir("t/io/empty_file");
    const int fd = osmium::io::detail::open_for_reading(input_file);
    REQUIRE(fd > 0);

    const int count2 = count_fds();
    osmium::io::GzipDecompressor decomp{fd};
    REQUIRE(decomp.read().empty());
    decomp.close();
    REQUIRE(count2 == count_fds());

    close(fd);
    REQUIRE(count1 == count_fds());
}

TEST_CASE("Read gzip-compressed file") {
    const int count1 = count_fds();

    const std::string input_file = with_data_dir("t/io/data_gzip.txt.gz");
    const int fd = osmium::io::detail::open_for_reading(input_file);
    REQUIRE(fd > 0);

    size_t size = 0;
    std::string all;
    {
        const int count2 = count_fds();
        osmium::io::GzipDecompressor decomp{fd};
        for (std::string data = decomp.read(); !data.empty(); data = decomp.read()) {
            size += data.size();
            all += data;
        }
        decomp.close();
        REQUIRE(count2 == count_fds());
    }

    REQUIRE(size >= 9);
    all.resize(8);
    REQUIRE("TESTDATA" == all);

    close(fd);
    REQUIRE(count1 == count_fds());
}

TEST_CASE("Read gzip-compressed file without explicit close") {
    const int count1 = count_fds();

    const std::string input_file = with_data_dir("t/io/data_gzip.txt.gz");
    const int fd = osmium::io::detail::open_for_reading(input_file);
    REQUIRE(fd > 0);

    size_t size = 0;
    std::string all;
    const int count2 = count_fds();
    {
        osmium::io::GzipDecompressor decomp{fd};
        for (std::string data = decomp.read(); !data.empty(); data = decomp.read()) {
            size += data.size();
            all += data;
        }
    }
    REQUIRE(count2 == count_fds());

    REQUIRE(size >= 9);
    all.resize(8);
    REQUIRE("TESTDATA" == all);

    close(fd);
    REQUIRE(count1 == count_fds());
}

TEST_CASE("Corrupted gzip-compressed file") {
    const int count1 = count_fds();

    const std::string input_file = with_data_dir("t/io/corrupt_data_gzip.txt.gz");
    const int fd = osmium::io::detail::open_for_reading(input_file);
    REQUIRE(fd > 0);

    const int count2 = count_fds();

    osmium::io::GzipDecompressor decomp{fd};
    decomp.read();
    REQUIRE_THROWS_AS(decomp.close(), const osmium::gzip_error&);

    REQUIRE(count2 == count_fds());

    close(fd);
    REQUIRE(count1 == count_fds());
}

