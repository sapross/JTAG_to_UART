#ifndef UARTDEVICE_H_
#define UARTDEVICE_H_

#include <string>
#include <termios.h>

class UARTDevice
{
  private:
    struct termios config;
    struct termios prev_config;

  public:
    int fd;
    UARTDevice(std::string term, unsigned int baudrate);
    ~UARTDevice();
    int         send(const std::string data);
    std::string receive(size_t num_bytes);
};

#endif // UARTDEVICE_H_
