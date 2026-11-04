#include "CommandHandler.hpp"

void CommandHandler::handlePING(int fd, const Command &cmd)
{
    std::string token;

    if (cmd.hasTrailing && !cmd.message.empty())
    {
        sendToClient(fd, "PONG :" + cmd.message + "\r\n");
        return;
    }
    if (!cmd.params.empty())
    {
        sendToClient(fd, "PONG " + cmd.params[0] + "\r\n");
        return;
    }
    if (cmd.hasTrailing)
    {
        sendToClient(fd, "PONG :" + cmd.message + "\r\n");
        return;
    }
    
    
    
    needMoreParams(fd, cmd.name);
}

void CommandHandler::handlePONG(int fd, const Command &cmd)
{
    (void)fd;
    (void)cmd;
    
}
