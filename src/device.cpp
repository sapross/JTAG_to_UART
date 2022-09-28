
#include "include/device.hpp"
#include <stdio.h>
#include <stdlib.h>

#include <cstring>
#include <fcntl.h>
#include <string>
#include <termios.h>
#include <unistd.h>

#include <iostream>

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

UARTDevice::UARTDevice(std::string term, unsigned int baudrate)
{
    fd = open(term.c_str(), O_RDWR | O_NOCTTY);
    if (not fd)
    {
        std::cout << "Notty" << std::endl;
        exit(1);
    }
    tcgetattr(fd, &(prev_config));
    bzero(&config, sizeof(config));
    cfmakeraw(&config);
    config.c_cflag |= baudrate;
    config.c_cc[VMIN] = 1;
    tcflush(fd, TCIOFLUSH);
    tcsetattr(fd, TCSANOW, &config);
}

UARTDevice::~UARTDevice()
{
    tcsetattr(fd, TCSANOW, &prev_config);
    close(fd);
}

int UARTDevice::send(const std::string data)
{

    std::cout << "Sending: '" << data << "'" << std::endl;
    if (not write(fd, data.c_str(), data.length()))
    {
        // writing failed
        std::cout << "Failure!" << std::endl;
        return 1;
    }
    tcdrain(fd);
    return 0;
}

std::string UARTDevice::receive(size_t num_bytes)
{
    char        buf;
    std::string data;
    for (size_t total = 0; total < num_bytes;)
    {
        size_t result = read(fd, &buf, 1);
        if (result)
        {
            data.push_back(buf);
            total++;
        }
        else
        {
            std::cout << "Read error!" << std::endl;
            break;
        }
    }
    std::cout << "Read: '" << data << "'" << std::endl;
    return data;
}

int main()
{
    std::string serial_device = "/dev/pts/7";
    UARTDevice  dev(serial_device, B115200);
    dev.send("Hello World!");
    dev.receive(3);
}
