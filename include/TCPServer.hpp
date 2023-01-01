#ifndef TCPSERVER_H_
#define TCPSERVER_H_

#include "JTAGDevice.hpp"
#include <thread>
class TCPServer
{
  public:
    bool debug;
    void start();
    void stop();
    void join();
    TCPServer(JTAGDevice& jtag, bool debug);
    ~TCPServer();

  private:
    int         fd;
    std::thread tid;
    JTAGDevice& jtag;
    void        thread_func();
};

#endif // TCPSERVER_H_
