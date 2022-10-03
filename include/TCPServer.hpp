#ifndef TCPSERVER_H_
#define TCPSERVER_H_

#include <thread>

class TCPServer
{
  public:
    void start();
    void stop();
    void join();
    TCPServer();
    ~TCPServer();

  private:
    int         fd;
    std::thread tid;
    void        thread_func();
};

#endif // TCPSERVER_H_
