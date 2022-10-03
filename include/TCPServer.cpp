#include "TCPServer.hpp"
#include "ConnectionHandler.hpp"

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

TCPServer::TCPServer()
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
    int sock_fd;
    std::cout << "Listen on port " << PORT << std::endl;
    if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        std::cerr << "socket failed. Error: " << errno << std::endl;
        return;
    }

    int reuse_addr = 1;
    if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &reuse_addr, sizeof(int)) == -1)
    {
        std::cerr << "setsockopt failed. Error: " << errno << std::endl;
        close(sock_fd);
        return;
    }

    struct sockaddr_in serv_addr = {0, 0, 0, 0};
    serv_addr.sin_family         = AF_INET;
    serv_addr.sin_addr.s_addr    = INADDR_ANY;
    serv_addr.sin_port           = htons(PORT);

    if (bind(sock_fd, (const struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
    {
        std::cerr << "bind failed. Error: " << errno << std::endl;
        close(sock_fd);
        return;
    }

    std::vector<struct pollfd> fds{{this->fd, POLLIN, 0}, {sock_fd, POLLIN, 0}};

    std::unordered_map<int, ConnectionHandler> handlers;

    while (true)
    {
        const int TIMEOUT = 1000;
        int       n       = poll(fds.data(), fds.size(), TIMEOUT);
        if (n == -1 && errno != ETIMEDOUT && errno != EINTR)
        {
            std::cerr << "poll failed. Error: " << errno << std::endl;
            break;
        }

        if (n > 0)
        {
            if (fds[0].revents)
            {
                std::cout << "Server stopping" << std::endl;
                break;
            }
            else if (fds[1].revents)
            {
                int client_fd = accept(sock_fd, NULL, NULL);
                std::cout << "New connection" << std::endl;
                if (client_fd != -1)
                {
                    fds.push_back(pollfd{client_fd, POLLIN, 0});

                    handlers.emplace(client_fd, client_fd);
                }
                else
                {
                    std::cerr << "accept failed. Error :" << errno << std::endl;
                }
                fds[1].revents = 0;
            }
            for (auto it = fds.begin() + 2; it != fds.end();)
            {
                char c;
                if (it->revents && recv(it->fd, &c, 1, MSG_PEEK | MSG_DONTWAIT) == 0)
                {
                    std::cout << "Client disconnected" << std::endl;
                    close(it->fd);
                    handlers.at(it->fd).terminate();
                    handlers.erase(it->fd);
                    it = fds.erase(it);
                }
                else
                {
                    it++;
                }
            }
        }
    }
    // cleaning section after receiving stop request
    for (auto it = fds.begin() + 1; it != fds.end(); it++)
    {
        close(it->fd);
        if (handlers.find(it->fd) != handlers.end())
        {
            handlers.at(it->fd).terminate();
        }
    }

    std::cout << "TCP server stopped" << std::endl;
}
