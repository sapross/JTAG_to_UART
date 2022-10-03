#ifndef ADAPTER_H_
#define ADAPTER_H_

#include <vector>

class Adapter
{
  public:
    Adapter();
    ~Adapter();
    int tap_reset();
    int get_ir(std::vector<bool>& ir);
    int exchange_ir(std::vector<bool>& ir);
    int get_dr(std::vector<bool>& dr);
    int exchange_dr(std::vector<bool>& dr);
};

#endif // ADAPTER_H_
