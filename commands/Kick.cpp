#include "CommandHandler.hpp"

void CommandHandler::handleKICK(int fd, const Command &cmd)
{
    if (cmd.params.size() < 2)
    {
        needMoreParams(fd, cmd.name);
        return;
    }

    Channel *channel = requireChannel(fd, cmd.params[0]);
    if (channel == NULL)
        return;

    if (!requireChannelOperator(fd, *channel))
        return;

    User *target = getUserByNick(cmd.params[1]);
    if (target == NULL || !channel->hasMember(target->getFd()))
    {
        sendNumeric(fd, IRC::ERR_USERNOTINCHANNEL,
                    cmd.params[1] + " " + channel->getName() +
                    " :They aren't on that channel");
        return;
    }

    User *kicker = getUser(fd);
    std::string reason = cmd.message.empty() ?
        kicker->getNickname() : cmd.message;

    std::string kickMsg = IRC::ResponseBuilder::clientMessage(
        kicker->getFullIdentifier(), "KICK",
        channel->getName() + " " + cmd.params[1], reason);

    broadcastToChannel(*channel, kickMsg);
    
    removeFromChannel(target->getFd(), *channel);
}
