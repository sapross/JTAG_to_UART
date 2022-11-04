#ifndef BITBANGHANDLER_H_
#define BITBANGHANDLER_H_
#include "JTAGDevice.hpp"

#include <thread>

class BitBangHandler
{
  private:
    std::thread m_thread;
    int         fd          = -1;
    bool        m_terminate = false;

    int  readMessage(std::string& msg_buffer);
    void sendMessage(const std::string& msg);
    void sendMessage(const uint8_t msg);

    JTAGDevice& jtag;

  public:
    explicit BitBangHandler(int fd, JTAGDevice& jtag);
    ~BitBangHandler();

    void stop();
    void terminate();
    void threadFunc();
};

#endif // BITBANGHANDLER_H_
