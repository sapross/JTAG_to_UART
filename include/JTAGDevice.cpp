#include "JTAGDevice.hpp"
#include "Adapter.hpp"

#include <algorithm>
#include <iostream>

bool rsh_bitvector(std::vector<bool>& bitvector, bool value)
{
    if (bitvector.size() > 0)
    {

        bool output = bitvector[0];
        for (size_t i = 0; i < bitvector.size() - 1; i++)
        {
            bitvector[i] = bitvector[i + 1];
        }
        bitvector[bitvector.size() - 1] = value;
        return output;
    }
    else
    {
        return false;
    }
}
bool lsh_bitvector(std::vector<bool>& bitvector, bool value)
{
    if (bitvector.size() > 0)
    {

        bool output = bitvector[bitvector.size() - 1];
        for (size_t i = bitvector.size(); i > 0; i--)
        {
            bitvector[i] = bitvector[i - 1];
        }
        bitvector[0] = value;
        return output;
    }
    else
    {
        return false;
    }
}

JTAGDevice::JTAGDevice(Adapter& adapter): adapter(adapter)
{
    this->state    = RUN_TEST_IDLE;
    this->tck_prev = false;
    this->ir       = std::vector<bool>(false, 5);
    this->output   = 0;
    this->ir_index = 0;
    this->dr_index = 0;
}
JTAGDevice::~JTAGDevice() { ; }

JTAG_STATES JTAGDevice::next_state(bool tms)
{
    switch (this->state)
    {
    case TEST_LOGIC_RESET: return tms ? TEST_LOGIC_RESET : RUN_TEST_IDLE;
    case RUN_TEST_IDLE: return tms ? SELECT_DR_SCAN : RUN_TEST_IDLE;
    case SELECT_DR_SCAN: return tms ? SELECT_IR_SCAN : CAPTURE_DR;
    case CAPTURE_DR: return tms ? EXIT1_DR : SHIFT_DR;
    case SHIFT_DR: return tms ? EXIT1_DR : SHIFT_DR;
    case EXIT1_DR: return tms ? UPDATE_DR : PAUSE_DR;
    case PAUSE_DR: return tms ? EXIT2_DR : PAUSE_DR;
    case EXIT2_DR: return tms ? UPDATE_DR : SHIFT_DR;
    case UPDATE_DR: return tms ? SELECT_DR_SCAN : RUN_TEST_IDLE;
    case SELECT_IR_SCAN: return tms ? TEST_LOGIC_RESET : CAPTURE_IR;
    case CAPTURE_IR: return tms ? EXIT1_IR : SHIFT_IR;
    case SHIFT_IR: return tms ? EXIT1_IR : SHIFT_IR;
    case EXIT1_IR: return tms ? UPDATE_IR : PAUSE_IR;
    case PAUSE_IR: return tms ? EXIT2_IR : PAUSE_IR;
    case EXIT2_IR: return tms ? UPDATE_IR : SHIFT_IR;
    case UPDATE_IR: return tms ? SELECT_DR_SCAN : RUN_TEST_IDLE;
    }
    return RUN_TEST_IDLE;
}
int JTAGDevice::proc_input(bool tck, bool tms, bool tdi)
{
    std::cout << "tck:" << tck << " tms:" << tms << " tdi:" << tdi << " tdo:" << this->output << std::endl;
    static bool _tms;
    if (this->tck_prev == 0 and tck == 1)
    {
        // Rising edge
        // Sample tdi and tms.
        _tms = tms;
        switch (this->state)
        {
        case TEST_LOGIC_RESET:
            std::cout << "JTAG: Test Logic Rest" << std::endl;
            this->adapter.tap_reset();
            break;
        case RUN_TEST_IDLE: // std::cout << "JTAG: Run Test Idle" << std::endl;
            break;
        case SELECT_DR_SCAN: // std::cout << "JTAG: Select DR Scan" << std::endl;
            break;
        case CAPTURE_DR:
            std::cout << "JTAG: Capture DR" << std::endl;
            this->adapter.get_dr(this->dr);
            this->output = this->dr.front() ? '1' : '0';
            // this->dr_index = 0;
            break;
        case SHIFT_DR:
            std::cout << "JTAG: Shift DR. tdi " << tdi << " :";
            print_bitvector(this->dr);
            std::cout << std::endl;
            rsh_bitvector(this->dr, tdi);
            this->output = this->dr.front() ? '1' : '0';
            // this->dr       = tdi;
            // this->dr_index = (this->dr_index + 1) % this->dr.size();
            break;
        case EXIT1_DR: // std::cout << "JTAG: Exit1 DR" << std::endl;
            break;
        case PAUSE_DR: // std::cout << "JTAG: Pause DR" << std::endl;
            break;
        case EXIT2_DR: // std::cout << "JTAG: Exit2 DR" << std::endl;
            break;
        case UPDATE_DR:
            std::cout << "JTAG: Update DR" << std::endl;
            this->adapter.exchange_dr(this->dr);
            break;
        case SELECT_IR_SCAN: // std::cout << "JTAG: Select IR" << std::endl;
            break;
        case CAPTURE_IR:
            std::cout << "JTAG: Capture IR" << std::endl;
            this->adapter.get_ir(this->ir);
            this->output = this->ir.front() ? '1' : '0';
            // this->ir_index = 0;
            break;
        case SHIFT_IR:
            std::cout << "JTAG: Shift IR. tdi " << tdi << " :";
            print_bitvector(this->ir);
            std::cout << std::endl;
            rsh_bitvector(this->ir, tdi);
            this->output = this->ir.front() ? '1' : '0';
            // this->output             = this->ir[this->ir_index] ? '1' : '0';
            // this->ir[this->ir_index] = tdi;
            // this->ir_index           = (this->ir_index + 1) % this->ir.size();
            break;
        case EXIT1_IR: // std::cout << "JTAG: Exit1 IR" << std::endl;
            break;
        case PAUSE_IR: // std::cout << "JTAG: Pause IR" << std::endl;
            break;
        case EXIT2_IR: // std::cout << "JTAG: Exit2 IR" << std::endl;
            break;
        case UPDATE_IR:
            std::cout << "JTAG: Update IR" << std::endl;
            this->adapter.exchange_ir(ir);
            break;
        }
    }
    if (this->tck_prev == 1 and tck == 0)
    {
        // Falling edge
        this->state = this->next_state(_tms);
    }

    this->tck_prev = tck;
    return 0;
}
