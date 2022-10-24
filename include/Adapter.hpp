#ifndef ADAPTER_H_
#define ADAPTER_H_
#include <vector>

#include "UARTdevice.hpp"

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
    NOP   = 0b00000000,
    READ  = 0b00100000,
    WRITE = 0b01000000,
    RW    = 0b01100000,
    RESET = 0b10000000
};

const uint8_t DEFAULT_ADDR = 0b00000001;
const uint8_t DTMCS_ADDR   = 0b00010000;
const uint8_t DMI_ADDR     = 0b00010001;
}; // namespace uart_tap

std::vector<bool> uint_to_bitvector(uint8_t value, size_t len);
uint8_t           bitvector_to_uint(std::vector<bool> bitvector);
std::vector<bool> string_to_bitvector(std::string str, size_t len);
std::string       bitvector_to_string(std::vector<bool> bitvector);
void              print_bitvector(std::vector<bool> bitvector);

class Adapter
{
  private:
    SerialDevice& uart;

  public:
    uint8_t address;

    Adapter(SerialDevice& uart);
    ~Adapter();
    int tap_reset();
    int get_ir(std::vector<bool>& ir);
    int exchange_ir(std::vector<bool>& ir);
    int get_dr(std::vector<bool>& dr);
    int exchange_dr(std::vector<bool>& dr);
};

#endif // ADAPTER_H_
