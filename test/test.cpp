#define CATCH_CONFIG_FAST_COMPILE
#include <algorithm>
#include <catch2/catch.hpp>
#include <iomanip>
#include <numeric>
#include <pty.h>
#include <random>
#include <sstream>
#include <termios.h>

#include "Adapter.hpp"
#include "UARTdevice.hpp"

TEST_CASE("UART Test")
{
    // Test parameters
    std::default_random_engine         generator;
    std::uniform_int_distribution<int> distribution(32, 126);
    auto                               randnum = std::bind(distribution, generator);

    int            master, slave;
    char           buf[256];
    struct termios tty;
    auto           serint = openpty(&master, &slave, buf, &tty, nullptr);

    UARTDevice uart(buf, 115200);

    // Tests
    SECTION("UART send test.")
    {
        std::string reference(50, 0);
        std::transform(
            reference.begin(), reference.end(), reference.begin(), [&randnum](auto a) { return (char)(randnum()); });
        uart.send(reference);
        char result[51];

        while (read(master, result, 50) < 0)
        {
            ;
        }
        REQUIRE_THAT(result, Catch::Matchers::Equals(reference));
    }

    SECTION("UART read test.")
    {
        std::string reference(50, 0);
        std::transform(
            reference.begin(), reference.end(), reference.begin(), [&randnum](auto a) { return (char)(randnum()); });

        write(master, reference.c_str(), 50);
        std::string result = uart.receive(50);
        REQUIRE_THAT(result, Catch::Matchers::Equals(reference));
    }
}
