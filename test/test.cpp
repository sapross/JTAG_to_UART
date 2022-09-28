#define CATCH_CONFIG_FAST_COMPILE
#include "logrange_generator.hpp"
#include <algorithm>
#include <catch2/catch.hpp>
#include <iomanip>
#include <numeric>
#include <random>
#include <sstream>

TEST_CASE("Example")
{
    // Test parameters
    size_t N = GENERATE(logRange(2, 1ull << 5, 2));
    // Logging of parameters
    CAPTURE(N);

    std::default_random_engine         generator;
    std::uniform_int_distribution<int> distribution(1, 10);
    auto                               randnum = std::bind(distribution, generator);

    int init  = 2;
    int ident = 0;

    std::vector<int> data(N, 1);
    std::generate(data.begin(), data.end(), randnum);

    std::vector<int> reference(N, 0);

    // Tests
    SECTION("Example Section")
    {
        std::vector<int> result(N, 0);
        REQUIRE_THAT(result, Catch::Matchers::Equals(reference));
    }
}
