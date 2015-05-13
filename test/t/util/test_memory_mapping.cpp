#include "catch.hpp"

#ifndef _MSC_VER
# include <unistd.h>
#else
# define ftruncate _chsize
#endif

#include <sys/types.h>

#include <osmium/util/memory_mapping.hpp>

#if defined(_MSC_VER) || (defined(__GNUC__) && defined(_WIN32))
#include "win_mkstemp.hpp"
#endif

TEST_CASE("anonymous mapping") {

    SECTION("simple memory mapping should work") {
        osmium::util::MemoryMapping mapping(1024);
        volatile int* addr = mapping.get_addr<int>();

        REQUIRE(mapping.writable());

        *addr = 42;
        REQUIRE(*addr == 42);

        REQUIRE(!!mapping);
        mapping.unmap();
        REQUIRE(!mapping);
        mapping.unmap(); // second unmap is okay
    }

    SECTION("memory mapping a huge area should fail") {
        const size_t huge = 1024ULL * 1024ULL * 1024ULL * 1024ULL;
        REQUIRE_THROWS_AS(osmium::util::MemoryMapping mapping(huge),
            std::system_error);
    }

    SECTION("moving a memory mapping should work") {
        osmium::util::MemoryMapping mapping1(1024);
        int* addr1 = mapping1.get_addr<int>();
        *addr1 = 42;

        REQUIRE(!!mapping1);
        osmium::util::MemoryMapping mapping2(std::move(mapping1));
        REQUIRE(!!mapping2);
        REQUIRE(!mapping1);
        mapping1.unmap();

        int* addr2 = mapping2.get_addr<int>();
        REQUIRE(*addr2 == 42);

        mapping2.unmap();
        REQUIRE(!mapping2);
    }

    SECTION("move assignment should work") {
        osmium::util::MemoryMapping mapping1(1024);
        osmium::util::MemoryMapping mapping2(1024);

        REQUIRE(!!mapping1);
        REQUIRE(!!mapping2);

        int* addr1 = mapping1.get_addr<int>();
        *addr1 = 42;

        mapping2 = std::move(mapping1);
        REQUIRE(!!mapping2);
        REQUIRE(!mapping1);

        int* addr2 = mapping2.get_addr<int>();
        REQUIRE(*addr2 == 42);

        mapping2.unmap();
        REQUIRE(!mapping2);
    }

#ifdef __linux__
    SECTION("remapping to larger size should work") {
        osmium::util::MemoryMapping mapping(1024);
        REQUIRE(mapping.size() == 1024);

        int* addr1 = mapping.get_addr<int>();
        *addr1 = 42;

        mapping.resize(2048);

        int* addr2 = mapping.get_addr<int>();
        REQUIRE(*addr2 == 42);
    }

    SECTION("remapping to smaller size should work") {
        osmium::util::MemoryMapping mapping(1024);
        REQUIRE(mapping.size() == 1024);

        int* addr1 = mapping.get_addr<int>();
        *addr1 = 42;

        mapping.resize(512);

        int* addr2 = mapping.get_addr<int>();
        REQUIRE(*addr2 == 42);
    }
#endif

    SECTION("writing to a mapped file should work") {
        const int size = 100;
        char filename[] = "test_mmap_file_size_XXXXXX";
        const int fd = mkstemp(filename);
        REQUIRE(fd > 0);

        REQUIRE(0 == ::ftruncate(fd, size));

        {
            osmium::util::MemoryMapping mapping(100, true, fd);
            REQUIRE(mapping.writable());

            REQUIRE(!!mapping);
            REQUIRE(mapping.size() == 100);

            *mapping.get_addr<int>() = 1234;

            mapping.unmap();
        }

        {
            osmium::util::MemoryMapping mapping(100, false, fd);
            REQUIRE(!mapping.writable());

            REQUIRE(!!mapping);
            REQUIRE(mapping.size() == 100);
            REQUIRE(*mapping.get_addr<int>() == 1234);

            mapping.unmap();
        }

        REQUIRE(0 == close(fd));
        REQUIRE(0 == unlink(filename));
    }

}

