#ifndef UARTDEVICE_H_
#define UARTDEVICE_H_

#include <string>
#include <termios.h>

unsigned int get_baudrate(int bd);
class SerialDevice
{
  public:
    virtual int         send(std::string data) { return 0; };
    virtual std::string receive(size_t num_bytes) { return std::string(); };
};

class UARTDevice: public SerialDevice
{
  private:
    struct termios config;
    struct termios prev_config;

  public:
    int fd;
    UARTDevice(std::string term, int baudrate);
    ~UARTDevice();
    int         send(std::string data);
    std::string receive(size_t num_bytes);
};

#endif // UARTDEVICE_H_
