#include "Server.hpp"

#include <iostream>
#include <new>

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        std::cerr << "Usage: ./ircserv <port> <password>" << std::endl;
        return 1;
    }
    try
    {
        Server server(argv[1], argv[2]);
        server.run();
    }
    catch (const std::bad_alloc &)
    {
        std::cerr << "Error: insufficient memory to start server" << std::endl;
        return 1;
    }
    return 0;
}