#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>
#include <vector>
#include <poll.h>
#include "CommandHandler.hpp"

class Client;

class Server
{
private:
    int                        port;
    std::string                password;
    std::string                portStr;
    int                        serverFd;
    std::vector<struct pollfd> pollFds;
    std::vector<Client *>      clients;
    CommandHandler             handler;

    void createSocket();
    void bindSocket();
    void listenSocket();
    void setNonBlocking(int fd);
    void addClient();
    void removeClient(int index);
    void receiveData(int index);
    void sendData(int index);
    void drainResponses();
    int  findClientIndexByFd(int fd) const;
    bool validatePort(const std::string &portStr);

public:
    Server(const std::string &port, const std::string &password);
    ~Server();
    void run();
};

#endif
