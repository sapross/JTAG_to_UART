#ifndef CONNECTIONHANDLER_H_
#define CONNECTIONHANDLER_H_

#include <thread>

class ConnectionHandler
{
  private:
    std::thread m_thread;
    int         fd          = -1;
    bool        m_terminate = false;

    std::string readMessage();
    void        sendMessage(const std::string& msg);

    void stop();

  public:
    explicit ConnectionHandler(int fd);
    ~ConnectionHandler();

    void terminate();
    void threadFunc();
};
#endif // CONNECTIONHANDLER_H_
