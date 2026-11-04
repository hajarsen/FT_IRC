#include "CommandHandler.hpp"

CommandHandler::CommandHandler()
    : _serverPassword("")
{
}

CommandHandler::~CommandHandler()
{
}

void CommandHandler::setServerPassword(const std::string &password)
{
    _serverPassword = password;
}

void CommandHandler::processLine(int fd, const std::string &line)
{
    if (line.empty())
        return;

    Command cmd = CommandParser::parse(line);

    if (!cmd.isValid())
    {
        sendNumeric(fd, IRC::ERR_UNKNOWNCOMMAND, ":Unknown command");
        return;
    }

    



    if (!cmd.prefix.empty())
        return;

    struct Entry
    {
        const char *name;
        void (CommandHandler::*handler)(int, const Command &);
        bool requiresRegistration;
    };
    static const Entry table[] = {
        {"PASS",    &CommandHandler::handlePASS,    false},
        {"NICK",    &CommandHandler::handleNICK,    false},
        {"USER",    &CommandHandler::handleUSER,    false},
        {"JOIN",    &CommandHandler::handleJOIN,    true},
        {"PRIVMSG", &CommandHandler::handlePRIVMSG, true},
        {"NOTICE",  &CommandHandler::handleNOTICE,  true},
        {"MODE",    &CommandHandler::handleMODE,    true},
        {"TOPIC",   &CommandHandler::handleTOPIC,   true},
        {"INVITE",  &CommandHandler::handleINVITE,  true},
        {"KICK",    &CommandHandler::handleKICK,    true},
        {"PART",    &CommandHandler::handlePART,    true},
        {"QUIT",    &CommandHandler::handleQUIT,    true},
        {"PING",    &CommandHandler::handlePING,    true},
        {"PONG",    &CommandHandler::handlePONG,    true},
    };
    static const std::size_t tableSize = sizeof(table) / sizeof(table[0]);

    const Entry *found = NULL;
    for (std::size_t i = 0; i < tableSize; ++i)
    {
        if (cmd.name == table[i].name)
        {
            found = &table[i];
            break;
        }
    }

    if (!found)
    {
        sendNumeric(fd, IRC::ERR_UNKNOWNCOMMAND, cmd.name + " :Unknown command");
        return;
    }

    User *user = getUser(fd);
    bool registered = (user != NULL && user->getIsRegistered());

    if (found->requiresRegistration && !registered)
    {
        sendNumeric(fd, IRC::ERR_NOTREGISTERED, ":You have not registered");
        return;
    }

    (this->*found->handler)(fd, cmd);
}

void CommandHandler::clientConnected(int fd)
{
    getOrCreateUser(fd);
}

void CommandHandler::clientDisconnected(int fd)
{
    _pendingDisconnect.erase(fd);
    User *user = getUser(fd);
    if (user != NULL && !user->getNickname().empty() && user->getIsRegistered())
    {
        std::string quitMsg = IRC::ResponseBuilder::clientMessage(
            user->getFullIdentifier(), "QUIT", "", "Connection closed");
        std::set<int> recipients;
        for (std::map<std::string, Channel>::iterator it = channels.begin();
             it != channels.end(); ++it)
        {
            Channel &ch = it->second;
            if (!ch.hasMember(fd))
                continue;
            const std::set<int> &members = ch.getMembers();
            for (std::set<int>::const_iterator m = members.begin(); m != members.end(); ++m)
            {
                if (*m != fd)
                    recipients.insert(*m);
            }
        }
        for (std::set<int>::const_iterator it = recipients.begin(); it != recipients.end(); ++it)
            sendToClient(*it, quitMsg);
    }
    
    removeFromAllChannels(fd);
}

void CommandHandler::resetSessions() throw()
{
    _responses.clear();
    _pendingDisconnect.clear();
    channels.clear();
    users.clear();
}

std::vector<int> CommandHandler::getMembersOfChannel(
    const std::string &channelName) const
{
    std::vector<int> result;

    const Channel *channel = findChannel(channelName);

    if (channel == NULL)
        return result;

    const std::set<int> &members = channel->getMembers();

    for (std::set<int>::const_iterator m = members.begin();
         m != members.end();
         ++m)
    {
        result.push_back(*m);
    }

    return result;
}

CommandHandler::DrainEffects CommandHandler::drain()
{
    DrainEffects eff;
    for (std::set<int>::const_iterator it = _pendingDisconnect.begin();
         it != _pendingDisconnect.end(); ++it)
        eff.disconnects.push_back(*it);
    for (std::map<int, std::string>::const_iterator it = _responses.begin();
         it != _responses.end(); ++it)
    {
        if (it->second.empty())
            continue;
        eff.replies[it->first] = it->second;
    }
    _responses.clear();
    _pendingDisconnect.clear();
    return eff;
}
void CommandHandler::sendToClient(int fd, const std::string &data)
{
    _responses[fd] += data;
}

void CommandHandler::sendNumeric(int fd, int code, const std::string &text)
{
    User *user = getUser(fd);

    std::string nick = "*";

    if (user != NULL && !user->getNickname().empty())
        nick = user->getNickname();

    sendToClient(fd, IRC::ResponseBuilder::numericResponse(code, nick, text));
}

void CommandHandler::broadcastToChannel(
    const Channel &channel,
    const std::string &message,
    int senderFd)
{
    const std::set<int> &members = channel.getMembers();

    for (std::set<int>::const_iterator m = members.begin();
         m != members.end(); ++m)
    {
        if (*m == senderFd)
            continue;

        sendToClient(*m, message);
    }
}

void CommandHandler::needMoreParams(int fd, const std::string &command)
{
    sendNumeric(fd, IRC::ERR_NEEDMOREPARAMS, command + " :Not enough parameters");
}

