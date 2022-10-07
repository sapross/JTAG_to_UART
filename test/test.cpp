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

TEST_CASE("UARTDevice Tests")
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
    SECTION("Send Message")
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

    SECTION("Receive Message")
    {
        std::string reference(50, 0);
        std::transform(
            reference.begin(), reference.end(), reference.begin(), [&randnum](auto a) { return (char)(randnum()); });

        write(master, reference.c_str(), 50);
        std::string result = uart.receive(50);
        REQUIRE_THAT(result, Catch::Matchers::Equals(reference));
    }
}

class DummyUART: public SerialDevice
{
  public:
    std::string msg;
    int         send(std::string data)
    {
        this->msg = data;
        return 0;
    }
    std::string receive(size_t num_bytes) { return msg; }
};

TEST_CASE("Adapter Tests")
{
    // Test parameters
    std::default_random_engine         generator;
    std::uniform_int_distribution<int> distribution(0, 255);
    auto                               randnum = std::bind(distribution, generator);

    DummyUART uart;
    Adapter   adapter(uart);
    // Tests
    SECTION("Adapter Reset")
    {

        std::vector<uint8_t> reference = {1, 128, 0, 0};
        adapter.tap_reset();
        std::vector<uint8_t> num_res(4, 0);
        for (size_t i = 0; i < 4; i++)
        {
            num_res[i] = (uint8_t)(uart.msg[i]);
        }
        REQUIRE_THAT(num_res, Catch::Matchers::Equals(reference));
    }

    SECTION("Adapter ir manip")
    {

        std::vector<bool> ref(uart_tap::IR_LENGTH, false);
        std::transform(ref.begin(), ref.end(), ref.begin(), [&randnum](auto a) { return randnum() < 127; });
        std::vector<bool> res(ref);

        adapter.exchange_ir(res);

        REQUIRE_THAT(uint_to_bitvector(uart_tap::DEFAULT_ADDR, uart_tap::IR_LENGTH), Catch::Matchers::Equals(res));
        REQUIRE_THAT(ref, Catch::Matchers::Equals(uint_to_bitvector(adapter.address, uart_tap::IR_LENGTH)));

        adapter.get_ir(res);

        REQUIRE_THAT(ref, Catch::Matchers::Equals(res));
    }

    // SECTION("Adapter dr manip")
    // {

    //     adapter.address                = uart_tap::DEFAULT_ADDR;
    //     std::vector<uint8_t> reference = {1, 128, 0, 0};

    //     std::vector<bool> ref(uart_tap::IDCODE_LENGTH, false);
    //     std::transform(ref.begin(), ref.end(), ref.begin(), [&randnum](auto a) { return randnum() < 127; });

    //     std::vector<bool> res(uart_tap::IDCODE_LENGTH, false);
    //     adapter.get_dr(res);

    //     std::vector<uint8_t> num_res(4, 0);
    //     for (size_t i = 0; i < 4; i++)
    //     {
    //         num_res[i] = (uint8_t)(uart.msg[i]);
    //     }

    //     REQUIRE_THAT(num_res, Catch::Matchers::Equals(res));
    //     REQUIRE_THAT(ref, Catch::Matchers::Equals(uint_to_bitvector(adapter.address, uart_tap::IR_LENGTH)));

    //     adapter.get_ir(res);

    //     REQUIRE_THAT(ref, Catch::Matchers::Equals(res));
    // }
}
