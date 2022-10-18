#include <stdio.h>
#include <stdlib.h>

#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <strings.h>
#include <termios.h>
#include <unistd.h>

#include "UARTdevice.hpp"

UARTDevice::UARTDevice(std::string term, unsigned int baudrate)
{
    if (term.size() > 0)
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
    else
    {
        fd = -1;
    }
}

UARTDevice::~UARTDevice()
{
    if (fd > 0)
    {
        tcsetattr(fd, TCSANOW, &prev_config);
        close(fd);
    }
}

int UARTDevice::send(std::string data)
{
    if (data.size() > 0)
    {
        if (not write(fd, data.c_str(), data.length()))
        {
            // writing failed
            std::cout << "Failure!" << std::endl;
            return 1;
        }
        tcdrain(fd);
        std::cout << "UART S: '";
        for (size_t i = 0; i < data.size(); i++)
        {
            std::cout << std::hex << +data[i] << ",";
        }
        std ::cout << "'" << std::endl;
    }
    return 0;
}

std::string UARTDevice::receive(size_t num_bytes)
{
    char        buf;
    std::string data;
    std::cout << "UART R: '";
    for (size_t total = 0; total < num_bytes;)
    {
        size_t result = read(fd, &buf, 1);
        if (result)
        {
            std::cout << std::hex << +buf << ",";
            data.push_back(buf);
            total++;
        }
        else
        {
            std::cout << "Read error!" << std::endl;
            break;
        }
    }
    std ::cout << "'" << std::endl;
    return data;
}
