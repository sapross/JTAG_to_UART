#include <stdio.h>
#include <stdlib.h>

#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <strings.h>
#include <termios.h>
#include <unistd.h>

#include "UARTdevice.hpp"

unsigned int get_baudrate(int bd)
{
    if (bd == 300)
    {
        return B300;
    }
    if (bd == 600)
    {
        return B600;
    }
    if (bd == 1200)
    {
        return B1200;
    }
    if (bd == 2400)
    {
        return B2400;
    }
    if (bd == 4800)
    {
        return B4800;
    }
    if (bd == 9600)
    {
        return B9600;
    }
    if (bd == 19200)
    {
        return B19200;
    }
    if (bd == 38400)
    {
        return B38400;
    }
    if (bd == 57600)
    {
        return B57600;
    }
    if (bd == 115200)
    {
        return B115200;
    }
    if (bd == 230400)
    {
        return B230400;
    }
    if (bd == 460800)
    {
        return B460800;
    }
    if (bd == 576000)
    {
        return B57600;
    }
    if (bd == 921600)
    {
        return B921600;
    }
    if (bd == 1000000)
    {
        return B1000000;
    }
    if (bd == 2000000)
    {
        return B2000000;
    }
    if (bd == 3000000)
    {
        return B3000000;
    }
    return 0;
}

UARTDevice::UARTDevice(std::string term, int baudrate)
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
        unsigned int bdcode = get_baudrate(baudrate);

        config.c_cflag |= bdcode;
        // Receive will terminate if either the desired number of characters is
        // received or a timeout of VTIME*0.1s is reached,
        config.c_cc[VMIN]  = 0;
        config.c_cc[VTIME] = 2;
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
