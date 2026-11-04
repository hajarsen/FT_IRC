#include "CommandHandler.hpp"

Channel *CommandHandler::findChannel(const std::string &name)
{
    std::map<std::string, Channel>::iterator it = channels.find(toLower(name));
    return it == channels.end() ? NULL : &it->second;
}

const Channel *CommandHandler::findChannel(const std::string &name) const
{
    std::map<std::string, Channel>::const_iterator it = channels.find(toLower(name));
    return it == channels.end() ? NULL : &it->second;
}

Channel *CommandHandler::requireChannel(int fd, const std::string &name)
{
    Channel *channel = findChannel(name);
    if (channel == NULL)
        sendNumeric(fd, IRC::ERR_NOSUCHCHANNEL, name + " :No such channel");
    return channel;
}

bool CommandHandler::requireChannelMember(int fd, Channel &channel)
{
    if (channel.hasMember(fd)) return true;
    sendNumeric(fd, IRC::ERR_NOTONCHANNEL,
                channel.getName() + " :You're not on that channel");
    return false;
}

bool CommandHandler::requireChannelOperator(int fd, Channel &channel)
{
    if (!requireChannelMember(fd, channel)) return false;
    if (channel.isOperator(fd)) return true;
    sendNumeric(fd, IRC::ERR_CHANOPRIVSNEEDED,
                channel.getName() + " :You're not channel operator");
    return false;
}

void CommandHandler::eraseChannelIfEmpty(const std::string &lowerName)
{
    std::map<std::string, Channel>::iterator it = channels.find(toLower(lowerName));
    if (it != channels.end() && it->second.isEmpty()) channels.erase(it);
}

void CommandHandler::addToChannel(int fd, Channel &channel)
{
    channel.addMember(fd);
    channel.removeInvited(fd);
}

void CommandHandler::removeFromChannel(int fd, Channel &channel)
{
    std::string key = toLower(channel.getName());
    channel.removeMember(fd);
    eraseChannelIfEmpty(key);
}

void CommandHandler::removeFromAllChannels(int fd)
{
    
    
    
    
    std::vector<std::string> toErase;
    for (std::map<std::string, Channel>::iterator it = channels.begin();
         it != channels.end(); ++it)
    {
        Channel &ch = it->second;
        if (ch.hasMember(fd))
        {
            ch.removeMember(fd);
            if (ch.isEmpty())
                toErase.push_back(it->first);
        }
        else if (ch.isInvited(fd))
            ch.removeInvited(fd);
    }
    for (std::size_t i = 0; i < toErase.size(); ++i)
    {
        std::map<std::string, Channel>::iterator it = channels.find(toErase[i]);
        if (it != channels.end() && it->second.isEmpty())
            channels.erase(it);
    }
    users.erase(fd);
}


User *CommandHandler::getUser(int fd)
{
    std::map<int, User>::iterator it = users.find(fd);
    return it == users.end() ? NULL : &it->second;
}

const User *CommandHandler::getUser(int fd) const
{
    std::map<int, User>::const_iterator it = users.find(fd);
    return it == users.end() ? NULL : &it->second;
}

User *CommandHandler::getOrCreateUser(int fd)
{
    User *user = getUser(fd);
    if (user == NULL) user = &users.insert(std::make_pair(fd, User(fd))).first->second;
    return user;
}

User *CommandHandler::getUserByNick(const std::string &nick)
{
    const std::string normalized = toLower(nick);
    for (std::map<int, User>::iterator it = users.begin(); it != users.end(); ++it)
        if (toLower(it->second.getNickname()) == normalized) return &it->second;
    return NULL;
}

std::string CommandHandler::toLower(const std::string &text)
{
    std::string result = text;
    for (std::string::size_type i = 0; i < result.size(); ++i)
    {
        unsigned char c = static_cast<unsigned char>(result[i]);
        if (c >= 'A' && c <= 'Z') result[i] = static_cast<char>(c - 'A' + 'a');
        else if (c == '{') result[i] = '[';
        else if (c == '}') result[i] = ']';
        else if (c == '|') result[i] = '\\';
        else if (c == '~') result[i] = '^';
    }
    return result;
}
