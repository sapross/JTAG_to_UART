#include "JTAGDevice.hpp"
#include "Adapter.hpp"

#include <algorithm>
#include <iostream>
JTAGDevice::JTAGDevice(Adapter& adapter): adapter(adapter)
{
    this->state      = RUN_TEST_IDLE;
    this->tck_prev   = false;
    this->ir         = std::vector<bool>(false, 5);
    this->output_buf = std::vector<bool>();
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
        // Rising edge
        this->state = this->next_state(tms);
        switch (this->state)
        {
        case TEST_LOGIC_RESET: this->adapter.tap_reset(); break;
        case RUN_TEST_IDLE: break;
        case SELECT_DR_SCAN: break;
        case CAPTURE_DR:
            this->adapter.get_dr(this->dr);
            this->dr_index = 0;
            break;
        case SHIFT_DR:
            if (this->dr.size() > 0)
            {

                this->output_buf.push_back(this->dr[this->dr_index]);
                this->dr[this->dr_index] = tdi;
                this->dr_index           = (this->dr_index + 1) % this->dr.size();
            }
            break;
        case EXIT1_DR: break;
        case PAUSE_DR: break;
        case EXIT2_DR: break;
        case UPDATE_DR: adapter.exchange_dr(this->dr); break;
        case SELECT_IR_SCAN: break;
        case CAPTURE_IR:
            this->adapter.get_ir(this->ir);
            this->ir_index = 0;
            break;
        case SHIFT_IR:
            break;
            for (auto it = this->output_buf.begin(); it != this->output_buf.end(); it++)
            {
                std::cout << *it;
            }
            std::cout << std::endl;
            this->output_buf.push_back(this->ir[this->ir_index]);
            for (auto it = this->output_buf.begin(); it != this->output_buf.end(); it++)
            {
                std::cout << *it;
            }
            std::cout << std::endl;
            this->ir[this->ir_index] = tdi;
            this->ir_index           = (this->ir_index + 1) % this->ir.size();
        case EXIT1_IR: break;
        case PAUSE_IR: break;
        case EXIT2_IR: break;
        case UPDATE_IR: adapter.exchange_ir(ir); break;
        }
    }

    this->tck_prev = tck;
    return 0;
}

std::string JTAGDevice::encode_output()
{

    std::string str(this->output_buf.size(), '0');
    for (size_t i = 0; i < output_buf.size(); i++)
    {
        str[i] = output_buf[i] ? '1' : '0';
    };
    std::cout << str << std::endl;
    return str;
}
