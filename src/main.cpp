#include <iostream>
#include <string>
#include <termios.h>

#include <lyra/lyra.hpp>

#include "Adapter.hpp"
#include "JTAGDevice.hpp"
#include "TCPServer.hpp"
#include "UARTdevice.hpp"

int main(int argc, char** argv)
{
    std::string tty      = "/dev/pts/7";
    int         baudrate = 115200;
    auto        cli      = lyra::cli() | lyra::opt(tty, "tty")["-t"]["--tty"]("Path to the serial device to use.") |
               lyra::opt(baudrate, "baudrate")["-b"]["--baudrate"]("Baudrate of target device.");

    auto result = cli.parse({argc, argv});
    if (!result)
    {
        std::cerr << "Error in command line: " << result.message() << std::endl;
        return 1;
    }
    unsigned int bd = get_baudrate(baudrate);
    if (bd == 0)
    {
        std::cerr << "Error: Invalid Baudrate " << baudrate << std::endl;
        return 1;
    }

    UARTDevice uart(tty, bd); // B3000000);
    Adapter    adapter(uart);
    JTAGDevice jtag(adapter);
    TCPServer  server(jtag);

    server.join();

    return 0;
}
