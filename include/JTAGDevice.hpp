#ifndef JTAGDEVICE_H_
#define JTAGDEVICE_H_

#include <string>
#include <termios.h>
#include <vector>

#include "Adapter.hpp"

enum JTAG_STATES
{
    TEST_LOGIC_RESET,
    RUN_TEST_IDLE,
    SELECT_DR_SCAN,
    CAPTURE_DR,
    SHIFT_DR,
    EXIT1_DR,
    PAUSE_DR,
    EXIT2_DR,
    UPDATE_DR,
    SELECT_IR_SCAN,
    CAPTURE_IR,
    SHIFT_IR,
    EXIT1_IR,
    PAUSE_IR,
    EXIT2_IR,
    UPDATE_IR,
};

class JTAGDevice
{
  private:
    bool        tck_prev;
    size_t      bit_count;
    Adapter&    adapter;
    JTAG_STATES state;
    JTAG_STATES next_state(bool tms);

    // Data register.
    std::vector<bool> dr;
    // Data register index
    size_t dr_index;
    // Instruction register
    std::vector<bool> ir;
    // Instruction register index
    size_t ir_index;

  public:
    uint8_t output;
    JTAGDevice(Adapter& adapter);
    ~JTAGDevice();
    int         proc_input(bool tck, bool tms, bool tdi);
    std::string encode_output();
};

#endif // JTAGDEVICE_H_
