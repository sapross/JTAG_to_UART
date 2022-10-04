#include "Adapter.hpp"

namespace uart_tap
{
const uint8_t HEADER = 0x01;
//
const size_t IR_LENGTH     = 5;             // bits
const size_t CMD_LENGTH    = 8 - IR_LENGTH; // bits
const size_t IDCODE_LENGTH = 32;            // bits
const size_t DTMCS_LENGTH  = 32;            // bits
const size_t DMI_LENGTH    = 41;            // bits

enum cmd_t
{
    NOP   = 0b0000000,
    READ  = 0b0010000,
    WRITE = 0b0100000,
    RW    = 0b0110000,
    RESET = 0b1000000
};

const uint8_t DEFAULT_ADDR = 0b00000001;
const uint8_t DTMCS_ADDR   = 0b00010000;
const uint8_t DMI_ADDR     = 0b00010001;
}; // namespace uart_tap

std::vector<bool> uint_to_bitvector(uint8_t value, size_t len)
{
    std::vector<bool> bitvector(len, false);
    for (size_t i = 0; i < bitvector.size(); i++)
    {
        // Convert address to bool vector.
        bitvector[i] = (1 << i) & value;
    }
    return bitvector;
}

uint8_t bitvector_to_uint(std::vector<bool> bitvector)
{
    uint8_t value = 0;
    for (size_t i = 0; i < bitvector.size(); i++)
    {
        // Convert address to bool vector.
        value += (1 << i) * bitvector[i];
    }
    return value;
}

std::vector<bool> string_to_bitvector(std::string str, size_t len)
{
    std::vector<bool> bitvector(len, false);
    for (size_t i = 0; i < bitvector.size(); i++)
    {
        // Convert address to bool vector.
        bitvector[i] = (1 << i % 8) & str[i / 8];
    }
    return bitvector;
}

std::string bitvector_to_string(std::vector<bool> bitvector)
{
    std::string str((bitvector.size() + 7 / 8), 0);
    for (size_t i = 0; i < bitvector.size(); i++)
    {
        // Convert address to bool vector.
        str[i / 8] = (1 << i % 8) * bitvector[i];
    }
    return str;
}

Adapter::Adapter(UARTDevice& uart): uart(uart) { this->address = uart_tap::DEFAULT_ADDR; }
Adapter::~Adapter() { ; }

int Adapter::tap_reset()
{
    std::string msg(3, 0);
    msg[0] = uart_tap::HEADER;
    msg[1] = uart_tap::RESET;
    return uart.send(msg);
}
int Adapter::get_ir(std::vector<bool>& ir)
{
    ir = uint_to_bitvector(this->address, uart_tap::IR_LENGTH);
    return 0;
}
int Adapter::exchange_ir(std::vector<bool>& ir)
{
    // Swap instruction register with address.
    auto temp     = uint_to_bitvector(this->address, uart_tap::IR_LENGTH);
    this->address = bitvector_to_uint(ir);
    ir            = temp;
    return 0;
}
int Adapter::get_dr(std::vector<bool>& dr)
{

    std::string msg(3, 0);
    msg[0] = uart_tap::HEADER;
    msg[1] = uart_tap::READ;

    size_t num_bits;
    switch (this->address)
    {
    case uart_tap::DEFAULT_ADDR: num_bits = uart_tap::IDCODE_LENGTH; break;
    case uart_tap::DTMCS_ADDR: num_bits = uart_tap::DTMCS_LENGTH; break;
    case uart_tap::DMI_ADDR: num_bits = uart_tap::DMI_LENGTH; break;
    default:
        // Otherwise we've hit a bypass register and read a zero byte.
        num_bits = 8;
        break;
    }
    size_t num_bytes = (num_bits + 7) / 8;
    msg[3]           = num_bytes;
    if (uart.send(msg))
    {
        std::string response = uart.receive(num_bytes);
        if (response.size() > 0)
        {
            dr = string_to_bitvector(response, num_bits);
            return num_bits;
        }
    }
    return -1;
}
int Adapter::exchange_dr(std::vector<bool>& dr)
{
    std::vector<bool> temp;
    if (!this->get_dr(temp))
    {
        return -1;
    }
    size_t      num_bytes = (temp.size() + 7) / 8;
    std::string msg(3, 0);
    msg[0] = uart_tap::HEADER;
    msg[1] = uart_tap::WRITE;
    msg[2] = num_bytes;
    msg += bitvector_to_string(dr);

    return this->uart.send(msg);
}
