#include "BitBangHandler.hpp"

#include <cassert>
#include <errno.h>
#include <iostream>
#include <pthread.h>
#include <sys/socket.h>

BitBangHandler::BitBangHandler(int fd, JTAGDevice& jtag): fd(fd), jtag(jtag)
{
    assert(!this->m_thread.joinable());

    // creating thread that handles received messages
    this->m_thread = std::thread([this]() { this->threadFunc(); });
    pthread_setname_np(this->m_thread.native_handle(), "BitBangHandler");
}

BitBangHandler::~BitBangHandler() { this->stop(); }

void BitBangHandler::stop()
{
    if (this->m_thread.joinable())
    {
        this->m_thread.join();
    }
}

int BitBangHandler::readMessage(std::string& msg_buffer)
{

    int readBytes = recv(this->fd, msg_buffer.data(), msg_buffer.size(), 0);
    if (readBytes < 1)
    {
        std::cout << "Error in readMessage, readBytes: " << readBytes << std::endl;
    }

    return readBytes;
}

void BitBangHandler::sendMessage(const std::string& msg)
{
    int n = send(this->fd, msg.c_str(), msg.size(), 0);
    if (n != static_cast<int>(msg.size()))
    {
        std::cout << "Error while sending message, message size: " << msg.size() << " bytes sent: " << std::endl;
    }
}

void BitBangHandler::sendMessage(const uint8_t msg)
{
    char buf[2];
    buf[0] = msg;
    buf[1] = 0;
    int n  = send(this->fd, buf, 1, 0);
    if (n != 1)
    {
        std::cout << "Error while sending message, message: " << std::hex << +msg << " bytes sent: " << std::endl;
    }
}

void BitBangHandler::terminate() { this->m_terminate = true; }

void BitBangHandler::threadFunc()
{
    while (!this->m_terminate)
    {
        std::string msg_buffer(1024, '\0'); // buffer with 1024 length which is filled with NULL character
                                            //
        int num_bytes = this->readMessage(msg_buffer);
        if (num_bytes < 0)
        {
            this->terminate();
        }
        else
        {
            // std::cout << "R: " << msg << std::endl;
            for (size_t i = 0; i < num_bytes; i++)
            {
                char symbol = msg_buffer[i];
                if (symbol == 'R')
                {
                    std::cout << "S: " << this->jtag.output << std::endl;
                    this->sendMessage(this->jtag.output);
                }
                else if (symbol == 'Q')
                {
                    this->terminate();
                }
                else
                {
                    switch (symbol)
                    {
                    case 'B': break;
                    case 'b': break;
                    case 'R': break;
                    case 'Q': break;
                    case '0': this->jtag.proc_input(false, false, false); break;
                    case '1': this->jtag.proc_input(false, false, true); break;
                    case '2': this->jtag.proc_input(false, true, false); break;
                    case '3': this->jtag.proc_input(false, true, true); break;
                    case '4': this->jtag.proc_input(true, false, false); break;
                    case '5': this->jtag.proc_input(true, false, true); break;
                    case '6': this->jtag.proc_input(true, true, false); break;
                    case '7': this->jtag.proc_input(true, true, true); break;
                    case 'r': break;
                    case 's': break;
                    case 't': break;
                    case 'u': break;
                    default: break;
                    }
                }
            }
        }
    }
}
