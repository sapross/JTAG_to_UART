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

        std::cout << "op=";
        if (to_uint(op[0]) == 0)
        {
            std::cout << "NOP  ";
        }
        else if (to_uint(op[0]) == 1)
        {
            std::cout << "READ ";
        }
        else if (to_uint(op[0]) == 2)
        {
            std::cout << "WRITE";
        }
        else
        {
            std::cout << "RES   ";
        }
        std::cout << ", ";
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

void print_bitvector_strhex(std::vector<bool> bitvector)
{
    std::string val = bitvector_to_string(bitvector);
    for (auto i = val.size(); i > 0; i--)
    {
        std::cout << std::setw(2) << std::setfill('0') << std::hex << to_uint(val[i - 1]);
    }
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
std::string insert_escape(std::string data)
{
    auto len = data.size();
    for (size_t i = 0; i < data.size(); i++)
    {
        if ((uint8_t)data[i] == uart_tap::ESCAPE)
        {
            len++;
        }
    }
    std::string new_data(len, 0);
    auto        it = data.begin();
    for (size_t i = 0; i < len;)
    {
        new_data[i] = *it;
        if ((uint8_t)*it == uart_tap::ESCAPE)
        {
            new_data[i + 1] = uart_tap::ESCAPE;
            i++;
        }
        i++;
        it++;
    }
    return new_data;
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
    std::string msg(2, 0);
    msg[0] = uart_tap::ESCAPE;
    msg[1] = uart_tap::RESET;
    uart.send(msg);
    msg = uart.receive(2);
    if (msg[0] == (char)(uart_tap::ESCAPE) and msg[1] == (char)(uart_tap::DEFAULT_ADDR))
    {
        return 0;
    }
    return 1;
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

    std::string msg(2, 0);
    msg[0] = uart_tap::ESCAPE;
    msg[1] = uart_tap::READ + this->address;

    size_t num_bits;
    switch (this->address)
    {
    case uart_tap::DEFAULT_ADDR: num_bits = uart_tap::IDCODE_LENGTH; break;
    case uart_tap::DTMCS_ADDR: num_bits = uart_tap::DTMCS_LENGTH; break;
    case uart_tap::DMI_ADDR: num_bits = uart_tap::DMI_LENGTH; break;
    case uart_tap::STB0_CS_ADDR: num_bits = uart_tap::STB0_CS_LENGTH; break;
    case uart_tap::STB0_D_ADDR: num_bits = uart_tap::STB0_D_LENGTH; break;
    case uart_tap::STB1_CS_ADDR: num_bits = uart_tap::STB1_CS_LENGTH; break;
    case uart_tap::STB1_D_ADDR: num_bits = uart_tap::STB1_D_LENGTH; break;
    default:
        // Otherwise we've hit a bypass register and read a zero byte.
        num_bits = 8;
        break;
    }
    size_t num_bytes = (num_bits + 7) / 8;
    // std::cout << "BitsBytes: " << num_bits << " " << num_bytes << std::endl;
    std::string response = "";
    uart.send(msg);
    std::string word = "";
    while (response.size() != num_bytes)
    {
        word = uart.receive(1);
        // Filter escape sequence and ignore instructions.
        // std::cout << word << std::endl;
        if ((uint8_t)(word[0]) == uart_tap::ESCAPE)
        {
            word = uart.receive(1);
            if ((uint8_t)(word[0]) == uart_tap::ESCAPE)
            {
                response += word;
            }
        }
        else
        {
            response += word;
        }
    }
    dr = string_to_bitvector(response, num_bits);
    if (this->debug)
    {
        std::cout << "get_dr = ";
        print_data_as_reg(dr);
        std::cout << ", raw = ";
        print_bitvector_strhex(dr);
        std::cout << std::endl;
    }
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
        std::string msg(2, 0);
        msg[0]           = uart_tap::ESCAPE;
        msg[1]           = uart_tap::WRITE + this->address;
        std::string data = insert_escape(bitvector_to_string(dr));
        msg.append(data);
        if (this->debug)
        {
            std::cout << "put_dr = ";
            print_data_as_reg(dr);
            std::cout << ", raw = ";
            print_bitvector_strhex(dr);
            std::cout << std::endl;
        }
        dr = temp;
        return this->uart.send(msg);
    }
    return 0;
}
