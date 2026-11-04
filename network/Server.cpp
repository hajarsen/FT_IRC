#include "Server.hpp"
#include "Client.hpp"

#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cctype>
#include <csignal>
#include <new>

static volatile sig_atomic_t g_running = 1;
static void handleSignal(int) { g_running = 0; }


bool Server::validatePort(const std::string &s)
{
    if (s.empty())
        return false;
    for (std::size_t i = 0; i < s.size(); ++i)
        if (!std::isdigit(static_cast<unsigned char>(s[i])))
            return false;
    char *end = NULL;
    long val = std::strtol(s.c_str(), &end, 10);
    if (end == NULL || *end != '\0')
        return false;
    if (val <= 0 || val > 65535)
        return false;
    port = static_cast<int>(val);
    return true;
}

Server::Server(const std::string &portStr_, const std::string &password_)
    : port(0), password(password_), portStr(portStr_), serverFd(-1)
{
    if (!validatePort(portStr))
    {
        std::cerr << "Error: invalid port '" << portStr << "'" << std::endl;
        std::exit(1);
    }
    handler.setServerPassword(password);
}

Server::~Server()
{
    for (std::size_t i = 0; i < clients.size(); ++i)
        delete clients[i];
    if (serverFd != -1)
        close(serverFd);
}

void Server::setNonBlocking(int fd)
{
    if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1)
    {
        std::cerr << "Error: fcntl() failed" << std::endl;
        std::exit(1);
    }
}

void Server::createSocket()
{
    serverFd = socket(AF_INET, SOCK_STREAM, 0);

    if (serverFd == -1)
    {
        std::cerr << "Error: socket() failed" << std::endl;
        std::exit(1);
    }
    int option = 1;
    if (setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)) == -1)
    {
        std::cerr << "Error: setsockopt() failed" << std::endl;
        close(serverFd);
        serverFd = -1;
        std::exit(1);
    }
    setNonBlocking(serverFd);
}

void Server::bindSocket()
{
    sockaddr_in address = sockaddr_in();

    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverFd,reinterpret_cast<sockaddr *>(&address), sizeof(address)) == -1)
    {
        std::cerr << "Error: bind() failed" << std::endl;
        close(serverFd);
        serverFd = -1;
        std::exit(1);
    }
}

void Server::listenSocket()
{
    if (listen(serverFd, 10) == -1)
    {
        std::cerr << "Error: listen() failed" << std::endl;
        close(serverFd);
        serverFd = -1;
        std::exit(1);
    }
}

void Server::addClient()
{
    int clientFd = accept(serverFd, NULL, NULL);

    if (clientFd == -1)
    {
        if (errno != EAGAIN && errno != EWOULDBLOCK)
            std::cerr << "Error: accept() failed" << std::endl;
        return;
    }
    setNonBlocking(clientFd);
    int sndBuf = 256 * 1024;
    int rcvBuf = 64 * 1024;
    setsockopt(clientFd, SOL_SOCKET, SO_SNDBUF, &sndBuf, sizeof(sndBuf));
    setsockopt(clientFd, SOL_SOCKET, SO_RCVBUF, &rcvBuf, sizeof(rcvBuf));
    Client *client = NULL;
    try
    {
        client = new Client(clientFd);
        clients.push_back(client);
    }
    catch (const std::bad_alloc &)
    {
        if (client != NULL)
            delete client;
        else
            close(clientFd);
        throw;
    }
    struct pollfd pollFd;
    pollFd.fd = clientFd;
    pollFd.events = POLLIN;
    pollFd.revents = 0;
    pollFds.push_back(pollFd);
    handler.clientConnected(clientFd);
    std::cout << "Client connected: " << clientFd << std::endl;
}

int Server::findClientIndexByFd(int fd) const
{
    for (std::size_t i = 0; i < clients.size(); ++i)
        if (clients[i]->getFd() == fd)
            return static_cast<int>(i);
    return -1;
}

void Server::drainResponses()
{
    CommandHandler::DrainEffects eff = handler.drain();
    std::vector<int> overflowFds;
    for (std::map<int, std::string>::iterator it = eff.replies.begin();
         it != eff.replies.end(); ++it)
    {
        int fd = it->first;
        const std::string &data = it->second;
        if (data.empty())
            continue;
        int idx = findClientIndexByFd(fd);
        if (idx == -1)
            continue;
        clients[idx]->enqueue(data);
        pollFds[idx + 1].events |= POLLOUT;
        bool overflow = false;
        if (clients[idx]->getOutBufferSize() > Client::MAX_OUT_BUFFER)
            overflow = true;
        if (overflow)
        {
            bool already = false;
            for (std::size_t k = 0; k < overflowFds.size(); ++k)
                if (overflowFds[k] == fd) { already = true; break; }
            if (!already)
                overflowFds.push_back(fd);
        }
    }
    for (std::size_t i = 0; i < eff.disconnects.size(); ++i)
    {
        int fd = eff.disconnects[i];
        int idx = findClientIndexByFd(fd);
        if (idx == -1)
            continue;
        clients[idx]->closeAfterFlush();
        pollFds[idx + 1].events = POLLOUT;
        if (!clients[idx]->hasOutput())
            removeClient(idx);
    }
    for (std::size_t i = 0; i < overflowFds.size(); ++i)
    {
        int fd = overflowFds[i];
        int idx = findClientIndexByFd(fd);
        if (idx == -1)
            continue;
        removeClient(idx);
    }
}

void Server::removeClient(int index)
{
    int fd = clients[index]->getFd();
    bool isOverflow = false;
    if (clients[index]->getOutBufferSize() > Client::MAX_OUT_BUFFER)
        isOverflow = true;
    std::cout << "Client disconnected: " << fd << (isOverflow ? " (SendQ exceeded)" : "") << std::endl;
    if (isOverflow)
    {
        struct linger ling;
        ling.l_onoff = 1;
        ling.l_linger = 0;
        setsockopt(fd, SOL_SOCKET, SO_LINGER, &ling, sizeof(ling));
    }
    handler.clientDisconnected(fd);
    CommandHandler::DrainEffects eff = handler.drain();
    std::vector<int> overflowFds;
    for (std::map<int, std::string>::iterator it = eff.replies.begin();
         it != eff.replies.end(); ++it)
    {
        int targetFd = it->first;
        const std::string &data = it->second;
        if (data.empty())
            continue;
        if (targetFd == fd)
            continue;
        int idx = findClientIndexByFd(targetFd);
        if (idx == -1)
            continue;
        clients[idx]->enqueue(data);
        pollFds[idx + 1].events |= POLLOUT;
        bool overflow = false;
        if (clients[idx]->getOutBufferSize() > Client::MAX_OUT_BUFFER)
            overflow = true;
        if (overflow)
        {
            bool already = false;
            for (std::size_t k = 0; k < overflowFds.size(); ++k)
                if (overflowFds[k] == targetFd) { already = true; break; }
            if (!already)
                overflowFds.push_back(targetFd);
        }
    }
    delete clients[index];
    clients.erase(clients.begin() + index);
    pollFds.erase(pollFds.begin() + index + 1);
    for (std::size_t i = 0; i < overflowFds.size(); ++i)
    {
        int ofd = overflowFds[i];
        int idx = findClientIndexByFd(ofd);
        if (idx == -1)
            continue;
        removeClient(idx);
    }
}

void Server::receiveData(int index)
{
    char buffer[65536];
    int fd = clients[index]->getFd();
    int bytesRead = recv(fd, buffer, sizeof(buffer), 0);
    if (bytesRead == 0)
    {
        clients[index]->closeAfterFlush();
        pollFds[index + 1].events = POLLOUT;
        if (!clients[index]->hasOutput())
            removeClient(index);
        return;
    }
    if (bytesRead < 0)
    {
        if (errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR)
            removeClient(index);
        return;
    }
    clients[index]->feed(buffer, static_cast<std::size_t>(bytesRead));
    std::vector<std::string> lines = clients[index]->extractLines();
    for (std::size_t i = 0; i < lines.size(); ++i)
    {
        handler.processLine(fd, lines[i]);
        drainResponses();
        if (findClientIndexByFd(fd) == -1)
            break;
        if (index >= static_cast<int>(clients.size()) || clients[index]->getFd() != fd)
            break;
        if (clients[index]->isClosing())
            break;
    }
}

void Server::sendData(int index)
{
    if (index < 0 || index >= static_cast<int>(clients.size()))
        return;
    if (!clients[index]->hasOutput())
    {
        pollFds[index + 1].events &= ~POLLOUT;
        return;
    }
    Client::FlushResult res = clients[index]->flush();
    if (res == Client::FlushError)
    {
        removeClient(index);
        return;
    }
    if (res == Client::FlushWouldBlock)
        return;
    if (clients[index]->isClosing())
    {
        removeClient(index);
        return;
    }
    if (clients[index]->hasOutput())
        pollFds[index + 1].events |= POLLOUT;
    else
        pollFds[index + 1].events &= ~POLLOUT;
}

void Server::run()
{
    g_running = 1;
    std::signal(SIGINT, handleSignal);
    std::signal(SIGQUIT, handleSignal);
    std::signal(SIGPIPE, SIG_IGN);

    createSocket();
    bindSocket();
    listenSocket();

    struct pollfd serverPollFd;
    serverPollFd.fd = serverFd;
    serverPollFd.events = POLLIN;
    serverPollFd.revents = 0;
    pollFds.push_back(serverPollFd);
    std::cout << "Server listening on port " << port << std::endl;

    while (g_running)
    {
        try
        {
            int result = poll(&pollFds[0], pollFds.size(), -1);
            if (result == -1)
            {
                if (errno == EINTR)
                    continue;
                if (errno == ENOMEM || errno == EAGAIN)
                    throw std::bad_alloc();
                std::cerr << "Error: poll() failed" << std::endl;
                break;
            }
            if (pollFds[0].revents & (POLLERR | POLLHUP | POLLNVAL))
            {
                std::cerr << "Error: server socket error" << std::endl;
                break;
            }
            if (pollFds[0].revents & POLLIN)
                addClient();

            for (int i = 0; i < static_cast<int>(clients.size()); ++i)
            {
                short revents = pollFds[i + 1].revents;
                if (revents == 0)
                    continue;
                if ((revents & POLLIN) && !clients[i]->isClosing())
                {
                    int fdBefore = clients[i]->getFd();
                    receiveData(i);
                    if (i >= static_cast<int>(clients.size()) ||
                        clients[i]->getFd() != fdBefore)
                    {
                        --i;
                        continue;
                    }
                }
                if (revents & (POLLHUP | POLLERR | POLLNVAL))
                {
                    removeClient(i);
                    --i;
                    continue;
                }
                if (pollFds[i + 1].revents & POLLOUT)
                {
                    int fdBefore = clients[i]->getFd();
                    sendData(i);
                    if (i >= static_cast<int>(clients.size()) ||
                        clients[i]->getFd() != fdBefore)
                    {
                        --i;
                        continue;
                    }
                }
            }
        }
        catch (const std::bad_alloc &)
        {
            
            
            for (std::size_t i = 0; i < clients.size(); ++i)
                delete clients[i];
            clients.clear();
            pollFds.resize(1);
            pollFds[0].revents = 0;
            handler.resetSessions();
            std::cerr << "Error: allocation failed; client sessions cleared" << std::endl;
        }
    }
    for (std::size_t i = 0; i < clients.size(); ++i)
        delete clients[i];
    clients.clear();
    pollFds.clear();
    if (serverFd != -1)
    {
        close(serverFd);
        serverFd = -1;
    }
}
