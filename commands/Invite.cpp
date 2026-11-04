#include "CommandHandler.hpp"

void CommandHandler::handleINVITE(int fd, const Command &cmd)
{
    if (cmd.params.size() < 2)
        return needMoreParams(fd, cmd.name);

    const std::string &nick = cmd.params[0];
    Channel *channel = requireChannel(fd, cmd.params[1]);

    if (!channel)
        return;
    if (channel->getModes().isInviteOnly()) {
        if (!requireChannelOperator(fd, *channel))
            return;
    } else {
        if (!requireChannelMember(fd, *channel))
            return;
    }

    User *target = getUserByNick(nick);

    if (!target)
        return sendNumeric(fd, IRC::ERR_NOSUCHNICKATTEMPT,
                           nick + " :No such nick");

    if (channel->hasMember(target->getFd()))
        return sendNumeric(fd, IRC::ERR_USERONCHANNEL,
                           nick + " " + channel->getName() +
                           " :is already on channel");

    channel->addInvited(target->getFd());

    sendNumeric(fd, IRC::RPL_INVITING,
                nick + " " + channel->getName());

    User *sender = getUser(fd);

    std::string msg = IRC::ResponseBuilder::clientMessage(
        sender->getFullIdentifier(),
        "INVITE",
        nick + " " + channel->getName(),
        "");

    sendToClient(target->getFd(), msg);
}
