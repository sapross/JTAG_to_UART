#ifndef ADAPTER_H_
#define ADAPTER_H_
#include <vector>

#include "UARTdevice.hpp"

class Adapter
{
  private:
    UARTDevice& uart;
    uint8_t     address;

  public:
    Adapter(UARTDevice& uart);
    ~Adapter();
    int tap_reset();
    int get_ir(std::vector<bool>& ir);
    int exchange_ir(std::vector<bool>& ir);
    int get_dr(std::vector<bool>& dr);
    int exchange_dr(std::vector<bool>& dr);
};

#endif // ADAPTER_H_
