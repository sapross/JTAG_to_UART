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
#include <numeric>

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

    unsigned int correct_seq = 0, faulty_seq = 0, incomplete_seq = 0;
    float        missing_char = 0.0, faulty_char = 0.0;

    for (size_t i = 0; i < num_seqences; i++)
    {
        std::transform(ref.begin(), ref.end(), ref.begin(), randnum);
        uart.send(ref);
        res = uart.receive(sequence_length);

        if (res.size() == ref.size())
        {
            float num_faults = 0.0;
            bool  faulty     = false;
            for (size_t j = 0; j < sequence_length; j++)
            {
                if (ref[j] != res[j])
                {
                    num_faults += 1.0;
                    faulty = true;
                }
            }
            if (!faulty)
            {
                correct_seq++;
            }
            else
            {
                faulty_char += num_faults / (float)(ref.size());
                faulty_seq++;
            }
        }
        else
        {
            // std::cout << "res.size(): " << res.size() << std::endl;
            missing_char += (float)(res.size()) / (float)(ref.size());
            incomplete_seq++;
        }
    }

    CAPTURE(correct_seq);
    CAPTURE(incomplete_seq);
    CAPTURE(faulty_seq);
    CAPTURE(missing_char);
    CAPTURE(faulty_char);

    REQUIRE(correct_seq == num_seqences);
}
