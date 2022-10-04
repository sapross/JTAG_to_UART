#include "TCPServer.hpp"
#include "BitBangHandler.hpp"

#include <cassert>
#include <iostream>
#include <netinet/in.h>
#include <poll.h>
#include <sys/eventfd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <unordered_map>
#include <vector>

#define PORT 1234

TCPServer::TCPServer(JTAGDevice& jtag): jtag(jtag)
{
    this->fd = -1;
    this->start();
}

TCPServer::~TCPServer()
{
    this->stop();
    if (this->fd != -1)
    {
        close(this->fd);
    }
}

void TCPServer::start()
{

    this->fd = eventfd(0, 0);
    if (this->fd == -1)
    {
        std::cerr << "eventfd failed. Error: " << errno << std::endl;
    }

    this->tid = std::thread(&TCPServer::thread_func, this);
    pthread_setname_np(this->tid.native_handle(), "TCPServer");
}

void TCPServer::stop()
{
    uint64_t one = 1;
    auto     ret = write(this->fd, &one, 8);
    if (ret == -1)
    {
        std::cerr << "write failed. Error: " << errno << std::endl;
    }
}

void TCPServer::join()
{
    if (this->tid.joinable())
    {
        this->tid.join();
    }
}

void TCPServer::thread_func()
{
    int sockfd;

    std::cout << "Listen on: " << PORT << std::endl;
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        std::cerr << "socket() => -1, errno=" << errno << std::endl;
        return;
    }

    int reuseaddr = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(int)) == -1)
    {
        std::cerr << "setsockopt() => -1, errno=" << errno << std::endl;
    }

    struct sockaddr_in servaddr = {0, 0, 0, 0};
    servaddr.sin_family         = AF_INET;
    servaddr.sin_addr.s_addr    = INADDR_ANY;
    servaddr.sin_port           = htons(PORT);

    // binding to socket that will listen for new connections
    if (bind(sockfd, (const struct sockaddr*)&servaddr, sizeof(servaddr)) == -1)
    {
        std::cerr << "bind() => -1, errno=" << errno << std::endl;
        close(sockfd);
        return;
    }

    // started listening, 50 pending connections can wait in a queue
    listen(sockfd, 50);

    // monitored file descriptors - at start there is efd and just created sockfd. POLLIN means we wait for data to read
    std::vector<struct pollfd> fds{{this->fd, POLLIN, 0}, {sockfd, POLLIN, 0}};

    BitBangHandler* handler;
    bool            handler_busy = false;
    while (true)
    {
        const int TIMEOUT = 1000;                      // 1000 ms
        int n = poll(fds.data(), fds.size(), TIMEOUT); // checking if there was any event on monitored file descriptors
        if (n == -1 && errno != ETIMEDOUT && errno != EINTR)
        {
            std::cerr << "poll() => -1, errno=" << errno << std::endl;
            break;
        }

        // n pending events
        if (n > 0)
        {
            if (fds[0].revents)
            { // handles server stop request (which is sent by TCPServer::stop())
                std::cout << "Received stop request" << std::endl;
                break;
            }
            else if (fds[1].revents)
            {
                // New connection.
                // Is there any other client connected already?
                if (!handler_busy)
                {
                    // accepting connection
                    int clientfd = accept(sockfd, NULL, NULL);
                    std::cout << "New connection" << std::endl;
                    if (clientfd != -1)
                    {
                        // insert new pollfd to monitor
                        fds.push_back(pollfd{clientfd, POLLIN, 0});

                        // create ConnectionHandler object that will run in separate thread
                        handler      = new BitBangHandler(clientfd, jtag);
                        handler_busy = true;
                    }
                    else
                    {
                        std::cerr << "accept => -1, errno=" << errno << std::endl;
                    }
                }
                // clearing revents
                fds[1].revents = 0;
            }

            // iterating all pollfds to check if anyone disconnected
            for (auto it = fds.begin() + 2; it != fds.end();)
            {
                char c;
                if (it->revents && recv(it->fd, &c, 1, MSG_PEEK | MSG_DONTWAIT) == 0)
                { // checks if disconnected or just fd readable
                    std::cout << "Client disconnected" << std::endl;
                    close(it->fd);        // closing socket
                    handler->terminate(); // terminating ConnectionHandler thread
                    delete handler;
                    handler_busy = false;
                    it           = fds.erase(it);
                }
                else
                {
                    ++it;
                }
            }
        }
    }

    // cleaning section after receiving stop request
    for (auto it = fds.begin() + 1; it != fds.end(); it++)
    {
        close(it->fd);
    }

    handler->terminate(); // terminating ConnectionHandler thread
    delete handler;

    std::cout << "TCP server stopped" << std::endl;
}
