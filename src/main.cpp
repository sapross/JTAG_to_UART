#include <iostream>
#include <string>
#include <termios.h>

#include "TCPServer.hpp"
#include "UARTdevice.hpp"

int main(int argc, char* argv[])
{
    // std::string serial_device = "/dev/pts/7";
    // UARTDevice  dev(serial_device, B115200);
    // dev.send("Hello World!");
    // dev.receive(3);

    TCPServer server;
    server.join();

    return 0;
}
