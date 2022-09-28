#include <iostream>
#include <string>
#include <termios.h>

#include "UARTdevice.hpp"

int main(int argc, char* argv[])
{
    std::string serial_device = "/dev/pts/7";
    UARTDevice  dev(serial_device, B115200);
    dev.send("Hello World!");
    dev.receive(3);
    return 0;
}
