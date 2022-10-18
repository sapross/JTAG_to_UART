#define CATCH_CONFIG_FAST_COMPILE
#include <algorithm>
#include <bitset>
#include <catch2/catch.hpp>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <pty.h>
#include <random>
#include <sstream>
#include <termios.h>

#include "UARTdevice.hpp"
#include "hw_test.hpp"

TEST_CASE("UART Echo Test")
{
    // Test parameters
    const unsigned int num_seqences    = 100;
    const unsigned int sequence_length = GENERATE(1, 2, 4, 8, 16, 32);

    CAPTURE(num_seqences);
    CAPTURE(sequence_length);

    std::default_random_engine         generator;
    std::uniform_int_distribution<int> distribution(32, 126);
    auto                               randnum = std::bind(distribution, generator);

    UARTDevice uart(tty, baudrate);

    std::string ref(sequence_length, 0);
    std::string res(sequence_length, 0);

    unsigned int hits = 0, misses = 0, faults = 0;

    for (size_t i = 0; i < num_seqences; i++)
    {
        std::transform(ref.begin(), ref.end(), ref.begin(), randnum);
        uart.send(ref);
        res = uart.receive(sequence_length);
        if (res.size() == ref.size())
        {
            if (std::equal(ref.begin(), ref.end(), res.begin(), res.end()))
            {
                hits++;
            }
            else
            {
                faults++;
            }
        }
        else
        {
            // std::cout << "res.size(): " << res.size() << std::endl;
            misses++;
        }
    }

    CAPTURE(misses);
    CAPTURE(faults);
    CAPTURE(hits);
    REQUIRE(hits == num_seqences);
}
