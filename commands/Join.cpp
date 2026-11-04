#include "CommandHandler.hpp"

void CommandHandler::handleJOIN(int fd, const Command &cmd)
{
    if (cmd.params.empty())
        return needMoreParams(fd, cmd.name);

    const std::string &channelParam = cmd.params[0];

    
    if (channelParam == "0")
    {
        User *user = getUser(fd);
        if (user == NULL)
            return;
        
        std::vector<std::string> toPartKeys;
        for (std::map<std::string, Channel>::iterator it = channels.begin();
             it != channels.end(); ++it)
        {
            if (it->second.hasMember(fd))
                toPartKeys.push_back(it->first);
        }
        for (std::size_t i = 0; i < toPartKeys.size(); ++i)
        {
            std::map<std::string, Channel>::iterator it = channels.find(toPartKeys[i]);
            if (it == channels.end())
                continue;
            Channel &channel = it->second;
            if (!channel.hasMember(fd))
                continue;
            std::string partMsg = IRC::ResponseBuilder::clientMessage(
                user->getFullIdentifier(), "PART", channel.getName(), "");
            broadcastToChannel(channel, partMsg);
            removeFromChannel(fd, channel);
        }
        return;
    }

    
    std::vector<std::string> targets;
    {
        std::string cur;
        for (std::size_t i = 0; i <= channelParam.size(); ++i)
        {
            if (i == channelParam.size() || channelParam[i] == ',')
            {
                if (!cur.empty())
                {
                    targets.push_back(cur);
                    cur.clear();
                }
            }
            else
                cur += channelParam[i];
        }
        if (targets.empty())
            targets.push_back(channelParam);
    }

    std::vector<std::string> keys;
    if (cmd.params.size() > 1)
    {
        const std::string &keyParam = cmd.params[1];
        std::string cur;
        for (std::size_t i = 0; i <= keyParam.size(); ++i)
        {
            if (i == keyParam.size() || keyParam[i] == ',')
            {
                
                
                
                
                
                
                keys.push_back(cur);
                cur.clear();
            }
            else
                cur += keyParam[i];
        }
        
        if (keys.size() == 1 && keys[0].empty())
            keys.clear();
        
        
        
        
        
    }

    for (std::size_t i = 0; i < targets.size(); ++i)
    {
        const std::string &name = targets[i];
        std::string key;
        if (i < keys.size())
            key = keys[i];
        else
            key = "";
        joinChannel(fd, name, key);
    }
}

void CommandHandler::joinChannel(int fd, const std::string &name,
                                 const std::string &key)
{
    if (!CommandParser::isValidChannelName(name))
        return sendNumeric(fd, IRC::ERR_NOSUCHCHANNEL,
                           name + " :No such channel");

    const std::string keyName = toLower(name);
    Channel *channel = findChannel(keyName);

    if (!channel)
    {
        std::pair<std::map<std::string, Channel>::iterator, bool> inserted =
            channels.insert(std::make_pair(keyName, Channel(name)));
        channel = &inserted.first->second;

        addToChannel(fd, *channel);
        channel->addOperator(fd);
    }
    else
    {
        if (channel->hasMember(fd))
            return;

        if (channel->getModes().isInviteOnly() &&
            !channel->isInvited(fd))
            return sendNumeric(fd, IRC::ERR_INVITEONLYCHAN,
                               name + " :Cannot join channel (+i)");

        if (channel->getModes().isKeyRequired() &&
            channel->getModes().getKey() != key)
            return sendNumeric(fd, IRC::ERR_BADCHANNELKEY,
                               name + " :Cannot join channel (+k)");

        if (channel->getModes().isUserLimitSet() &&
            channel->getMemberCount() >= channel->getModes().getUserLimit())
            return sendNumeric(fd, IRC::ERR_CHANNELISFULL,
                               name + " :Cannot join channel (+l)");

        addToChannel(fd, *channel);
    }

    User *user = getUser(fd);

    std::string msg = IRC::ResponseBuilder::clientMessage(
        user->getFullIdentifier(), "JOIN", channel->getName(), "");

    broadcastToChannel(*channel, msg);

    if (!channel->getTopic().empty())
        sendNumeric(fd, IRC::RPL_TOPIC,
                    channel->getName() + " :" + channel->getTopic());

    std::string names;
    const std::set<int> &members = channel->getMembers();
    for (std::set<int>::const_iterator it = members.begin(); it != members.end(); ++it)
    {
        User *member = getUser(*it);
        if (!member)
            continue;
        std::string nick = member->getNickname();
        if (channel->isOperator(*it))
            nick = "@" + nick;
        if (!names.empty())
            names += " ";
        names += nick;
    }
    sendNumeric(fd, IRC::RPL_NAMREPLY, "= " + channel->getName() + " :" + names);
    sendNumeric(fd, IRC::RPL_ENDOFNAMES, channel->getName() + " :End of /NAMES list.");
}
