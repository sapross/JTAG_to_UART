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

    std::string readMessage();
    void        sendMessage(const std::string& msg);
    void        stop();

    JTAGDevice& jtag;

  public:
    explicit BitBangHandler(int fd, JTAGDevice& jtag);
    ~BitBangHandler();

    void terminate();
    void threadFunc();
};

#endif // BITBANGHANDLER_H_
