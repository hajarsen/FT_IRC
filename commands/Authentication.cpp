#include "CommandHandler.hpp"

void CommandHandler::tryRegister(int fd)
{
    User *u = getUser(fd);

    if (!u || u->getIsRegistered() ||
        (!_serverPassword.empty() && !u->getIsAuthenticated()) ||
        u->getNickname().empty() || u->getUsername().empty())
        return;

    u->setIsRegistered(true);
    sendWelcome(fd);
}

void CommandHandler::sendWelcome(int fd)
{
    User *u = getUser(fd);

    if (!u)
        return;

    sendNumeric(fd, IRC::RPL_WELCOME,
        ":Welcome to the Internet Relay Network " + u->getFullIdentifier());
    sendNumeric(fd, IRC::RPL_YOURHOST,
        ":Your host is " + IRC::serverName() + ", running version 1.0");
    sendNumeric(fd, IRC::RPL_CREATED,
        ":This server was created for the 42 ft_irc project");
    sendNumeric(fd, IRC::RPL_MYINFO, IRC::serverName() + " 1.0 o o");
}

void CommandHandler::handlePASS(int fd, const Command &cmd)
{
    User *u = getOrCreateUser(fd);

    if (u->getIsRegistered())
        return sendNumeric(fd, IRC::ERR_ALREADYREGISTERED,
                           ":You may not reregister");

    if (cmd.params.empty() || cmd.params[0].empty())
        return needMoreParams(fd, cmd.name);

    if (cmd.params[0] != _serverPassword)
        return sendNumeric(fd, IRC::ERR_PASSWDMISMATCH,
                           ":Password incorrect");

    u->setIsAuthenticated(true);
    tryRegister(fd);
}

void CommandHandler::handleNICK(int fd, const Command &cmd)
{
    User *u = getOrCreateUser(fd);

    if (!_serverPassword.empty() && !u->getIsAuthenticated())
        return sendNumeric(fd, IRC::ERR_PASSWDMISMATCH, ":Password incorrect");

    std::string nick;
    if (!cmd.params.empty() && !cmd.params[0].empty())
        nick = cmd.params[0];
    else if (cmd.hasTrailing && !cmd.message.empty())
        nick = cmd.message;
    else
        return sendNumeric(fd, IRC::ERR_NONICKNAMEGIVEN,
                           ":No nickname given");

    if (!CommandParser::isValidNickname(nick))
        return sendNumeric(fd, IRC::ERR_ERRONEUSNICKNAME,
                           nick + " :Erroneous nickname");

    if (toLower(nick) != toLower(u->getNickname()) &&
        getUserByNick(nick))
        return sendNumeric(fd, IRC::ERR_NICKNAMEINUSE,
                           nick + " :Nickname is already in use");

    const std::string oldNick = u->getNickname();
    u->setNickname(nick);

    if (!oldNick.empty() && toLower(oldNick) != toLower(nick))
    {
        std::string msg =
            IRC::ResponseBuilder::clientMessage(oldNick, "NICK", "", nick);

        std::set<int> recipients;
        recipients.insert(fd);
        for (std::map<std::string, Channel>::iterator it = channels.begin();
             it != channels.end(); ++it)
        {
            Channel &ch = it->second;
            if (!ch.hasMember(fd))
                continue;
            const std::set<int> &members = ch.getMembers();
            for (std::set<int>::const_iterator m = members.begin(); m != members.end(); ++m)
                recipients.insert(*m);
        }
        for (std::set<int>::const_iterator it = recipients.begin(); it != recipients.end(); ++it)
            sendToClient(*it, msg);
    }

    tryRegister(fd);
}

void CommandHandler::handleUSER(int fd, const Command &cmd)
{
    User *u = getOrCreateUser(fd);

    if (!_serverPassword.empty() && !u->getIsAuthenticated())
        return sendNumeric(fd, IRC::ERR_PASSWDMISMATCH, ":Password incorrect");

    if (!u->getUsername().empty())
        return sendNumeric(fd, IRC::ERR_ALREADYREGISTERED,
                           ":You may not reregister");

    if (cmd.params.size() < 3)
        return needMoreParams(fd, cmd.name);

    std::string realname = cmd.message;

    if (realname.empty() && cmd.params.size() >= 4)
    {
        realname = cmd.params[3];

        for (std::size_t i = 4; i < cmd.params.size(); ++i)
            realname += " " + cmd.params[i];
    }

    if (realname.empty())
        return needMoreParams(fd, cmd.name);

    u->setUsername(cmd.params[0]);
    u->setRealname(realname);
    if (u->getNickname().empty())
        return sendNumeric(fd, IRC::ERR_NONICKNAMEGIVEN,
                           ":No nickname given");
    tryRegister(fd);
}
