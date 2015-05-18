#include "catch.hpp"

#ifndef _MSC_VER
# include <unistd.h>
#else
# define ftruncate _chsize
#endif

#include <sys/types.h>
#include <limits>

#include <osmium/util/memory_mapping.hpp>

#if defined(_MSC_VER) || (defined(__GNUC__) && defined(_WIN32))
#include "win_mkstemp.hpp"
#endif

static const size_t huge = std::numeric_limits<size_t>::max();

TEST_CASE("anonymous mapping") {

    SECTION("simple memory mapping should work") {
        osmium::util::MemoryMapping mapping(1024);
        REQUIRE(mapping.get_addr() != nullptr);

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

}

TEST_CASE("file-based mapping") {

    SECTION("writing to a mapped file should work") {
        char filename[] = "test_mmap_write_XXXXXX";
        const int fd = mkstemp(filename);
        REQUIRE(fd > 0);

        REQUIRE(0 == ::ftruncate(fd, 100));

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

    SECTION("remapping to larger size should work") {
        char filename[] = "test_mmap_grow_XXXXXX";
        const int fd = mkstemp(filename);
        REQUIRE(fd > 0);

        REQUIRE(0 == ::ftruncate(fd, 200));

        osmium::util::MemoryMapping mapping(100, true, fd);
        REQUIRE(mapping.size() == 100);

        int* addr1 = mapping.get_addr<int>();
        *addr1 = 42;

        mapping.resize(200);
        REQUIRE(mapping.size() == 200);

        int* addr2 = mapping.get_addr<int>();
        REQUIRE(*addr2 == 42);

        mapping.unmap();

        REQUIRE(0 == close(fd));
        REQUIRE(0 == unlink(filename));
    }

    SECTION("remapping to smaller size should work") {
        char filename[] = "test_mmap_shrink_XXXXXX";
        const int fd = mkstemp(filename);
        REQUIRE(fd > 0);

        REQUIRE(0 == ::ftruncate(fd, 100));

        {
            osmium::util::MemoryMapping mapping(100, true, fd);
            REQUIRE(mapping.size() == 100);

            int* addr1 = mapping.get_addr<int>();
            *addr1 = 42;

            mapping.resize(50);
            REQUIRE(mapping.size() == 50);

            int* addr2 = mapping.get_addr<int>();
            REQUIRE(*addr2 == 42);
        }

        REQUIRE(0 == close(fd));
        REQUIRE(0 == unlink(filename));
    }
}

TEST_CASE("typed anonymous mapping") {

    SECTION("simple memory mapping should work") {
        osmium::util::TypedMemoryMapping<uint32_t> mapping(1024);
        volatile uint32_t* addr = mapping.get_addr();

        REQUIRE(mapping.writable());

        *addr = 42;
        REQUIRE(*addr == 42);

        REQUIRE(!!mapping);
        mapping.unmap();
        REQUIRE(!mapping);
        mapping.unmap(); // second unmap is okay
    }

    SECTION("memory mapping a huge area should fail") {
        REQUIRE_THROWS_AS(osmium::util::TypedMemoryMapping<uint32_t> mapping(huge),
            std::system_error);
    }

    SECTION("moving a memory mapping should work") {
        osmium::util::TypedMemoryMapping<uint32_t> mapping1(1024);
        uint32_t* addr1 = mapping1.get_addr();
        *addr1 = 42;

        REQUIRE(!!mapping1);
        osmium::util::TypedMemoryMapping<uint32_t> mapping2(std::move(mapping1));
        REQUIRE(!!mapping2);
        REQUIRE(!mapping1);
        mapping1.unmap();

        auto addr2 = mapping2.get_addr();
        REQUIRE(*addr2 == 42);

        mapping2.unmap();
        REQUIRE(!mapping2);
    }

    SECTION("move assignment should work") {
        osmium::util::TypedMemoryMapping<uint32_t> mapping1(1024);
        osmium::util::TypedMemoryMapping<uint32_t> mapping2(1024);

        REQUIRE(!!mapping1);
        REQUIRE(!!mapping2);

        auto addr1 = mapping1.get_addr();
        *addr1 = 42;

        mapping2 = std::move(mapping1);
        REQUIRE(!!mapping2);
        REQUIRE(!mapping1);

        auto addr2 = mapping2.get_addr();
        REQUIRE(*addr2 == 42);

        mapping2.unmap();
        REQUIRE(!mapping2);
    }

#ifdef __linux__
    SECTION("remapping to larger size should work") {
        osmium::util::TypedMemoryMapping<uint32_t> mapping(1024);
        REQUIRE(mapping.size() == 1024);

        auto addr1 = mapping.get_addr();
        *addr1 = 42;

        mapping.resize(2048);

        auto addr2 = mapping.get_addr();
        REQUIRE(*addr2 == 42);
    }

    SECTION("remapping to smaller size should work") {
        osmium::util::TypedMemoryMapping<uint32_t> mapping(1024);
        REQUIRE(mapping.size() == 1024);

        auto addr1 = mapping.get_addr();
        *addr1 = 42;

        mapping.resize(512);

        auto addr2 = mapping.get_addr();
        REQUIRE(*addr2 == 42);
    }
#endif

}

TEST_CASE("typed file-based mapping") {

    SECTION("writing to a mapped file should work") {
        char filename[] = "test_mmap_file_size_XXXXXX";
        const int fd = mkstemp(filename);
        REQUIRE(fd > 0);

        REQUIRE(0 == ::ftruncate(fd, 100));

        {
            osmium::util::TypedMemoryMapping<uint32_t> mapping(100, true, fd);
            REQUIRE(mapping.writable());

            REQUIRE(!!mapping);
            REQUIRE(mapping.size() == 100);

            *mapping.get_addr() = 1234;

            mapping.unmap();
        }

        {
            osmium::util::TypedMemoryMapping<uint32_t> mapping(100, false, fd);
            REQUIRE(!mapping.writable());

            REQUIRE(!!mapping);
            REQUIRE(mapping.size() == 100);
            REQUIRE(*mapping.get_addr() == 1234);

            mapping.unmap();
        }

        REQUIRE(0 == close(fd));
        REQUIRE(0 == unlink(filename));
    }

}

TEST_CASE("anonymous memory mapping class") {

    SECTION("simple memory mapping should work") {
        osmium::util::AnonymousMemoryMapping mapping(1024);
        REQUIRE(mapping.get_addr() != nullptr);

        volatile int* addr = mapping.get_addr<int>();

        REQUIRE(mapping.writable());

        *addr = 42;
        REQUIRE(*addr == 42);

        REQUIRE(!!mapping);
        mapping.unmap();
        REQUIRE(!mapping);
        mapping.unmap(); // second unmap is okay
    }

#ifdef __linux__
    SECTION("remapping to larger size should work") {
        osmium::util::AnonymousMemoryMapping mapping(1024);
        REQUIRE(mapping.size() == 1024);

        int* addr1 = mapping.get_addr<int>();
        *addr1 = 42;

        mapping.resize(2048);

        int* addr2 = mapping.get_addr<int>();
        REQUIRE(*addr2 == 42);
    }

    SECTION("remapping to smaller size should work") {
        osmium::util::AnonymousMemoryMapping mapping(1024);
        REQUIRE(mapping.size() == 1024);

        int* addr1 = mapping.get_addr<int>();
        *addr1 = 42;

        mapping.resize(512);

        int* addr2 = mapping.get_addr<int>();
        REQUIRE(*addr2 == 42);
    }
#endif

}

