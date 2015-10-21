#include "catch.hpp"

#include <chrono>
#include <stdexcept>
#include <thread>

#include <osmium/thread/pool.hpp>

struct test_job_with_result {
    int operator()() const {
        return 42;
    }
};

struct test_job_throw {
    void operator()() const {
        throw std::runtime_error("exception in pool thread");
    }
};

TEST_CASE("thread") {

    auto& pool = osmium::thread::Pool::instance();

    SECTION("can get access to thread pool") {
        REQUIRE(pool.queue_empty());
    }

    SECTION("can send job to thread pool") {
        auto future = pool.submit(test_job_with_result {});

        REQUIRE(future.get() == 42);
    }

    SECTION("can throw from job in thread pool") {
        auto future = pool.submit(test_job_throw {});

        REQUIRE_THROWS_AS(future.get(), std::runtime_error);
    }

}

