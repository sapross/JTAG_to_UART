#include "JTAGDevice.hpp"
#include "Adapter.hpp"
JTAGDevice::JTAGDevice()
{
    FSM      = RUN_TEST_IDLE;
    tck_prev = false;
    ir       = std::vector<bool>(false, 5);
}
JTAGDevice::~JTAGDevice() { ; }

JTAG_STATES JTAGDevice::fsm_next(bool tms)
{
    switch (FSM)
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

    if (tck_prev == 0 and tck == 1)
    {
        // Rising edge
        FSM = fsm_next(tms);
        switch (FSM)
        {
        case TEST_LOGIC_RESET: adapter.tap_reset(); break;
        case RUN_TEST_IDLE: break;
        case SELECT_DR_SCAN: break;
        case CAPTURE_DR:
            dr_i = 0;
            adapter.get_dr(dr);
            break;
        case SHIFT_DR:
            proc_output(dr[dr_i]);
            dr[dr_i] = tdi;
            dr_i++;
            break;
        case EXIT1_DR: break;
        case PAUSE_DR: break;
        case EXIT2_DR: break;
        case UPDATE_DR: adapter.exchange_dr(dr); break;
        case SELECT_IR_SCAN: break;
        case CAPTURE_IR:
            ir_i = 0;
            adapter.get_ir(ir);
            break;
        case SHIFT_IR:
            break;
            proc_output(ir[ir_i]);
            ir[ir_i] = tdi;
            ir_i++;
        case EXIT1_IR: break;
        case PAUSE_IR: break;
        case EXIT2_IR: break;
        case UPDATE_IR: adapter.exchange_ir(ir); break;
        }
    }

    tck_prev = tck;
    return 0;
}
