#include "Adapter.hpp"

#include <iomanip>
#include <iostream>

// Helper function to get correct output through std:hex;
unsigned int to_uint(char val) { return (unsigned int)((uint8_t)(val)); }

void Adapter::print_data_as_reg(std::vector<bool> dr)
{

    switch (this->address)
    {
    case uart_tap::DMI_ADDR:
    {
        // Acquire fields.
        auto        start = dr.begin();
        std::string op    = bitvector_to_string(start, start + 2);
        start += 2;
        std::string data = bitvector_to_string(start, start + 32);
        start += 32;
        std::string address = bitvector_to_string(start, start + 7);
        start += 7;
        std::cout << "DMI: ";
        // Print fields
        std::cout << "addr=" << std::setw(2) << std::setfill('0') << std::hex << to_uint(address[0]);
        std::cout << ", ";
        std::cout << "op=" << std::setw(2) << std::setfill('0') << std::hex << to_uint(op[0]) << ", ";
        std::cout << "data=";
        for (auto i = data.size(); i > 0; i--)
        {
            std::cout << std::setw(2) << std::setfill('0') << std::hex << to_uint(data[i - 1]);
        }
        break;
    }
    case uart_tap::DEFAULT_ADDR:
    {
        std::string val = bitvector_to_string(dr);
        std::cout << "IDCODE=";
        for (auto i = val.size(); i > 0; i--)
        {
            std::cout << std::setw(2) << std::setfill('0') << std::hex << to_uint(val[i - 1]);
        }
        break;
    }
    case uart_tap::DTMCS_ADDR:
    {
        std::string val = bitvector_to_string(dr);
        std::cout << "DTMCS=";
        for (auto i = val.size(); i > 0; i--)
        {
            std::cout << std::setw(2) << std::setfill('0') << std::hex << to_uint(val[i - 1]);
        }
        break;
    }
    default:
    {
        // Register at default address is 32 bit.
        std::string val = bitvector_to_string(dr);
        std::cout << "BYPASS=";
        for (auto i = val.size(); i > 0; i--)
        {
            std::cout << std::setw(2) << std::setfill('0') << std::hex << to_uint(val[i - 1]);
        }
        break;
    }
    }
    std::cout << std::endl;
}

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
    // OpenOCD processes data in little endian, while TAP communicates in big endian.
    // Hence, process bytes in reverse.
    auto it = str.begin();
    for (size_t j = 0; j < len; j++)
    {
        char c;
        if (j % 8 == 0)
        {
            c = *it;
            it++;
        }
        bitvector[j] = (1 << j % 8) & c;
    }

    return bitvector;
}

std::string bitvector_to_string(std::vector<bool> bitvector)
{
    std::string str((bitvector.size() + 7) / 8, 0);
    // Convert endianness between TAP and OpenOCD
    auto it = str.begin();
    for (size_t i = 0; i < bitvector.size(); i++)
    {
        if (i > 0 && i % 8 == 0)
        {
            it++;
        }
        *it += (1 << i % 8) * bitvector[i];
    }
    return str;
}
std::string bitvector_to_string(std::vector<bool>::iterator begin, std::vector<bool>::iterator end)
{
    auto        size = end - begin;
    std::string str((size + 7) / 8, 0);
    // Convert endianness between TAP and OpenOCD
    auto it = str.begin();
    for (size_t i = 0; i < size; i++)
    {
        if (i > 0 && i % 8 == 0)
        {
            it++;
        }
        *it += (1 << i % 8) * (*(begin + i));
    }
    return str;
}
void print_bitvector(std::vector<bool> bitvector)
{
    for (size_t i = bitvector.size(); i > 0; i--)
    {
        std::cout << (bitvector[i - 1] ? '1' : '0');
    }
}

Adapter::Adapter(SerialDevice& uart, bool debug): uart(uart), debug(debug) { this->address = uart_tap::DEFAULT_ADDR; }
Adapter::~Adapter() { ; }

int Adapter::tap_reset()
{
    this->address = uart_tap::DEFAULT_ADDR;
    std::string msg(3, 0);
    msg[0] = uart_tap::HEADER;
    msg[1] = uart_tap::RESET;
    return uart.send(msg);
}
int Adapter::get_ir(std::vector<bool>& ir)
{
    ir = uint_to_bitvector(this->address, uart_tap::IR_LENGTH);
    // std::cout << "get_ir : ir = ";
    // print_bitvector(ir);
    // std::cout << std::endl;
    return 0;
}
int Adapter::exchange_ir(std::vector<bool>& ir)
{
    // Swap instruction register with address.
    auto temp     = uint_to_bitvector(this->address, uart_tap::IR_LENGTH);
    this->address = bitvector_to_uint(ir);
    // std::cout << "exchange_ir : ir_in = ";
    // print_bitvector(ir);
    // std::cout << " , ir_out = ";
    // print_bitvector(temp);
    // std::cout << std::endl;
    ir = temp;
    return 0;
}
int Adapter::get_dr(std::vector<bool>& dr)
{

    std::string msg(3, 0);
    msg[0] = uart_tap::HEADER;
    msg[1] = uart_tap::READ + this->address;

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
    msg[2]           = num_bytes;
    // std::cout << "BitsBytes: " << num_bits << " " << num_bytes << std::endl;
    std::string response;
    size_t      num_attempts = 0;
    while (response.size() != num_bytes)
    {
        if (num_attempts < 3)
        {

            uart.send(msg);
            response = uart.receive(num_bytes);
        }
        else
        {
            return -1;
        }
    }
    dr = string_to_bitvector(response, num_bits);
    // std::cout << "get_dr : dr = ";
    // print_bitvector(dr);
    // std::cout << std::endl;
    return num_bits;
}
int Adapter::exchange_dr(std::vector<bool>& dr)
{
    std::vector<bool> temp;
    if (!this->get_dr(temp))
    {
        return -1;
    }
    if (dr.size() > 0)
    {
        size_t      num_bytes = (temp.size() + 7) / 8;
        std::string msg(3, 0);
        msg[0] = uart_tap::HEADER;
        msg[1] = uart_tap::WRITE + this->address;
        msg[2] = num_bytes;
        msg.append(bitvector_to_string(dr));
        if (this->debug)
        {
            print_data_as_reg(dr);
            std::cout << "exchhange_dr : dr_in = ";
            print_bitvector(dr);
            std::cout << " , dr_out = ";
            print_bitvector(temp);
            std::cout << std::endl;
        }
        dr = temp;
        return this->uart.send(msg);
    }
    return 0;
}
