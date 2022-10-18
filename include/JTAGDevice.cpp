#include "JTAGDevice.hpp"
#include "Adapter.hpp"

#include <algorithm>
#include <iostream>
JTAGDevice::JTAGDevice(Adapter& adapter): adapter(adapter)
{
    this->state    = RUN_TEST_IDLE;
    this->tck_prev = false;
    this->ir       = std::vector<bool>(false, 5);
    this->output   = 0;
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
    if (this->tck_prev == 0 and tck == 1)
    {
        // //std::cout << "tck: posedge "
        //           << " tms: " << tms << " tdi: " << tdi << std::endl;
        // Rising edge
        this->state = this->next_state(tms);
        switch (this->state)
        {
        case TEST_LOGIC_RESET:
            // std::cout << "JTAG: Test Logic Rest" << std::endl;
            this->adapter.tap_reset();
            break;
        case RUN_TEST_IDLE: // std::cout << "JTAG: Run Test Idle" << std::endl;
            break;
        case SELECT_DR_SCAN: // std::cout << "JTAG: Select DR Scan" << std::endl;
            break;
        case CAPTURE_DR:
            // std::cout << "JTAG: Capture DR" << std::endl;
            this->adapter.get_dr(this->dr);
            this->dr_index = 0;
            break;
        case SHIFT_DR:
            // std::cout << "JTAG: Shift DR" << std::endl;
            if (this->dr.size() > 0)
            {
                if (this->dr_index == this->dr.size())
                {
                    // We completed shifting one full register but want to shift in more data.
                    // Exchange dr to update TAP and get new contents.
                    this->adapter.exchange_dr(this->dr);
                    this->dr_index = 0;
                }
                this->output             = this->dr[this->dr_index] ? '1' : '0';
                this->dr[this->dr_index] = tdi;
                this->dr_index++;
            }
            break;
        case EXIT1_DR: // std::cout << "JTAG: Exit1 DR" << std::endl;
            break;
        case PAUSE_DR: // std::cout << "JTAG: Pause DR" << std::endl;
            break;
        case EXIT2_DR: // std::cout << "JTAG: Exit2 DR" << std::endl;
            break;
        case UPDATE_DR:
            // std::cout << "JTAG: Update DR" << std::endl;
            this->adapter.exchange_dr(this->dr);
            break;
        case SELECT_IR_SCAN: // std::cout << "JTAG: Select IR" << std::endl;
            break;
        case CAPTURE_IR:
            // std::cout << "JTAG: Capture IR" << std::endl;
            this->adapter.get_ir(this->ir);
            this->ir_index = 0;
            break;
        case SHIFT_IR:
            // std::cout << "JTAG: Shift IR" << std::endl;
            if (this->ir_index == this->ir.size())
            {
                // We completed shifting one full register but want to shift in more data.
                // Exchange ir to update Adapter address and get new contents.
                this->adapter.exchange_ir(this->ir);
                this->ir_index = 0;
            }
            this->output             = this->ir[this->ir_index] ? '1' : '0';
            this->ir[this->ir_index] = tdi;
            this->ir_index++;
            break;
        case EXIT1_IR: // std::cout << "JTAG: Exit1 IR" << std::endl;
            break;
        case PAUSE_IR: // std::cout << "JTAG: Pause IR" << std::endl;
            break;
        case EXIT2_IR: // std::cout << "JTAG: Exit2 IR" << std::endl;
            break;
        case UPDATE_IR:
            // std::cout << "JTAG: Update IR" << std::endl;
            this->adapter.exchange_ir(ir);
            break;
        }
    }

    this->tck_prev = tck;
    return 0;
}
