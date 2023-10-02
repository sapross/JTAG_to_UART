#ifndef ADAPTER_H_
#define ADAPTER_H_
#include <cstdint>
#include <vector>

#include "UARTdevice.hpp"

namespace uart_tap
{
const uint8_t ESCAPE = 0b10100000;
//
const size_t IR_LENGTH      = 5;             // bits
const size_t CMD_LENGTH     = 8 - IR_LENGTH; // bits
const size_t IDCODE_LENGTH  = 32;            // bits
const size_t DTMCS_LENGTH   = 32;            // bits
const size_t DMI_LENGTH     = 41;            // bits
const size_t STB0_CS_LENGTH = 32;            // bits
const size_t STB0_D_LENGTH  = 32;            // bits
const size_t STB1_CS_LENGTH = 8;             // bits
const size_t STB1_D_LENGTH  = 8;             // bits

enum cmd_t
{
    NOP       = 0b00000000,
    READ      = 0b00100000,
    CONT_READ = 0b01000000,
    WRITE     = 0b01100000,
    RESET     = 0b11100000
};

const uint8_t ADDR_NOP     = 0b00000000;
const uint8_t DEFAULT_ADDR = 0b00000001;
const uint8_t DTMCS_ADDR   = 0b00010000;
const uint8_t DMI_ADDR     = 0b00010001;
const uint8_t STB0_CS_ADDR = 0b00010100;
const uint8_t STB0_D_ADDR  = 0b00010101;
const uint8_t STB1_CS_ADDR = 0b00010110;
const uint8_t STB1_D_ADDR  = 0b00010111;
}; // namespace uart_tap

std::vector<bool> uint_to_bitvector(uint8_t value, size_t len);
uint8_t           bitvector_to_uint(std::vector<bool> bitvector);
std::vector<bool> string_to_bitvector(std::string str, size_t len);
std::string       bitvector_to_string(std::vector<bool> bitvector);
std::string       bitvector_to_string(std::vector<bool>::iterator begin, std::vector<bool>::iterator end);
void              print_bitvector(std::vector<bool> bitvector);

class Adapter
{
  private:
    SerialDevice& uart;
    void          print_data_as_reg(std::vector<bool> ir);

  public:
    uint8_t address;
    bool    debug;
    Adapter(SerialDevice& uart, bool debug);
    ~Adapter();
    int tap_reset();
    int get_ir(std::vector<bool>& ir);
    int exchange_ir(std::vector<bool>& ir);
    int get_dr(std::vector<bool>& dr);
    int exchange_dr(std::vector<bool>& dr);
};

#endif // ADAPTER_H_
