#include "JTAGDevice.hpp"
#include "Adapter.hpp"

#include <algorithm>
JTAGDevice::JTAGDevice(Adapter& adapter): adapter(adapter)
{
    this->state    = RUN_TEST_IDLE;
    this->tck_prev = false;
    this->ir       = std::vector<bool>(false, 5);
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
            this->dr_it = this->dr.begin();
            break;
        case SHIFT_DR:
            this->output_buf.emplace_back(*this->dr_it);
            *this->dr_it = tdi;
            this->dr_it++;
            break;
        case EXIT1_DR: break;
        case PAUSE_DR: break;
        case EXIT2_DR: break;
        case UPDATE_DR: adapter.exchange_dr(this->dr); break;
        case SELECT_IR_SCAN: break;
        case CAPTURE_IR:
            this->adapter.get_ir(this->ir);
            this->ir_it = this->ir.begin();
            break;
        case SHIFT_IR:
            break;
            this->output_buf.emplace_back(*this->ir_it);
            *this->ir_it = tdi;
            this->ir_it++;
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

    std::string output(this->output_buf.size(), '0');
    auto        it  = output.begin();
    auto        beg = this->output_buf.begin();
    auto        end = this->output_buf.end();
    std::for_each(beg,
                  end,
                  [&it](auto& a)
                  {
                      *it = (a ? '1' : '0');
                      it++;
                  });
    return output;
}
