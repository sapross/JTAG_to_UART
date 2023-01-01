#define CATCH_CONFIG_FAST_COMPILE
#include <algorithm>
#include <catch2/catch.hpp>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <pty.h>
#include <random>
#include <sstream>
#include <strings.h>
#include <termios.h>

#include "Adapter.hpp"
#include "UARTdevice.hpp"

TEST_CASE("UARTDevice Tests")
{
    // Test parameters
    std::default_random_engine         generator;
    std::uniform_int_distribution<int> distribution(32, 126);
    auto                               randnum = std::bind(distribution, generator);

    int  master, slave;
    char buf[256];

    unsigned int   bdcode = get_baudrate(115200);
    struct termios config;
    bzero(&config, sizeof(config));
    cfmakeraw(&config);
    config.c_cflag |= bdcode;
    // Read will return immediatly, whether data is available or not.
    config.c_cc[VMIN]  = 0;
    config.c_cc[VTIME] = 0;
    auto serint        = openpty(&master, &slave, buf, &config, nullptr);

    UARTDevice uart(buf, 115200, true);

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
    std::string response;
    int         send(std::string data)
    {
        // std::cout << "S(" << data.size() << "): ";
        // for (size_t i = 0; i < data.size(); i++)
        // {
        //     std::cout << (uint16_t)(data[i]) << " ";
        // }
        // std::cout << std::endl;
        this->msg = data;
        return 0;
    }
    std::string receive(size_t num_bytes)
    {
        // std::cout << "R(" << this->response.size() << "): ";
        // for (size_t i = 0; i < this->response.size(); i++)
        // {
        //     std::cout << (uint16_t)(this->response[i]) << " ";
        // }
        // std::cout << std::endl;
        return response;
    }
};

TEST_CASE("Adapter Tests")
{
    // Test parameters
    std::default_random_engine         generator;
    std::uniform_int_distribution<int> distribution(0, 255);
    auto                               randnum = std::bind(distribution, generator);

    DummyUART uart;
    Adapter   adapter(uart, true);
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

    SECTION("Adapter dr manip")
    {

        std::vector<bool> dmi(uart_tap::DMI_LENGTH, false);
        std::transform(dmi.begin(), dmi.end(), dmi.begin(), [&randnum](auto a) { return randnum() < 127; });
        uart.response = bitvector_to_string(dmi);

        std::vector<bool> dr;

        adapter.address = uart_tap::DMI_ADDR;
        adapter.get_dr(dr);
        // SOH,CMD_READ+DMI_ADDR,num_bytes,0
        std::string ref_msg = "\x01\x31\x06";

        REQUIRE_THAT(uart.msg, Catch::Matchers::Equals(ref_msg));
        REQUIRE_THAT(dr, Catch::Matchers::Equals(dmi));

        std::vector<bool> new_dmi(uart_tap::DMI_LENGTH, false);
        std::transform(new_dmi.begin(), new_dmi.end(), new_dmi.begin(), [&randnum](auto a) { return randnum() < 127; });
        ref_msg = "\x01\x51\x06" + bitvector_to_string(new_dmi);

        adapter.exchange_dr(new_dmi);
        REQUIRE_THAT(new_dmi, Catch::Matchers::Equals(dmi));

        REQUIRE_THAT(uart.msg, Catch::Matchers::Equals(ref_msg));
        REQUIRE_THAT(dr, Catch::Matchers::Equals(dmi));
    }
}
