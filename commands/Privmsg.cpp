#include "CommandHandler.hpp"
#include "bonus.hpp"

void CommandHandler::handlePRIVMSG(int fd, const Command &cmd)
{
    sendMessageToTarget(fd, cmd, false);
}

void CommandHandler::handleNOTICE(int fd, const Command &cmd)
{
    sendMessageToTarget(fd, cmd, true);
}

void CommandHandler::sendMessageToTarget(int fd, const Command &cmd, bool isNotice)
{
    if (cmd.params.empty() || cmd.message.empty())
    {
        if (isNotice)
            return;
        if (cmd.params.empty())
            sendNumeric(fd, IRC::ERR_NORECIPIENT, ":No recipient given");
        else
            sendNumeric(fd, IRC::ERR_NOTEXTTOSEND, ":No text to send");
        return;
    }

    User *sender = getUser(fd);
    if (sender == NULL)
        return;

    std::string commandName = isNotice ? "NOTICE" : "PRIVMSG";
    const std::string &rawTarget = cmd.params[0];

    
    std::vector<std::string> targets;
    std::string cur;
    for (std::size_t i = 0; i <= rawTarget.size(); ++i)
    {
        if (i == rawTarget.size() || rawTarget[i] == ',')
        {
            if (!cur.empty())
            {
                targets.push_back(cur);
                cur.clear();
            }
        }
        else
            cur += rawTarget[i];
    }
    if (targets.empty())
        targets.push_back(rawTarget);

    for (std::size_t i = 0; i < targets.size(); ++i)
    {
        const std::string &target = targets[i];
        if (target.empty())
            continue;
        if (target[0] == '#' || target[0] == '&')
        {
            Channel *channel = findChannel(target);
            if (channel == NULL)
            {
                if (isNotice)
                    continue;
                sendNumeric(fd, IRC::ERR_NOSUCHCHANNEL, target + " :No such channel");
                continue;
            }
            if (!channel->hasMember(fd))
            {
                if (isNotice)
                    continue;
                sendNumeric(fd, IRC::ERR_CANNOTSENDTOCHAN, target + " :Cannot send to channel");
                continue;
            }
            std::string message = IRC::ResponseBuilder::clientMessage(
                sender->getFullIdentifier(), commandName, target, cmd.message);
            broadcastToChannel(*channel, message, fd);
        }
        else
        {
            User *recipient = getUserByNick(target);
            if (recipient != NULL)
            {
                std::string message = IRC::ResponseBuilder::clientMessage(
                    sender->getFullIdentifier(), commandName, target, cmd.message);
                sendToClient(recipient->getFd(), message);
            }
            else if (Bot::isTarget(target))
            {
                
                
                if (isNotice)
                    continue;
                std::string message = IRC::ResponseBuilder::clientMessage(
                    "bot!bot@localhost", commandName,
                    sender->getNickname(), Bot::reply(sender->getNickname(), cmd.message));
                sendToClient(fd, message);
            }
            else
            {
                if (isNotice)
                    continue;
                sendNumeric(fd, IRC::ERR_NOSUCHNICKATTEMPT, target + " :No such nick");
            }
        }
    }
}
