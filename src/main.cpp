#include <iostream>
#include <string>
#include <termios.h>

#include "Adapter.hpp"
#include "JTAGDevice.hpp"
#include "TCPServer.hpp"
#include "UARTdevice.hpp"

int main(int argc, char* argv[])
{
    std::string serial_device = "/dev/pts/7";
    UARTDevice  uart(serial_device, B115200);
    Adapter     adapter(uart);
    JTAGDevice  jtag(adapter);
    TCPServer   server(jtag);

    server.join();

    return 0;
}
