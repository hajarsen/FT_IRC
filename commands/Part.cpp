#include "CommandHandler.hpp"

void CommandHandler::handlePART(int fd, const Command &cmd)
{
    if (cmd.params.empty())
        return needMoreParams(fd, cmd.name);

    User *user = getUser(fd);
    if (user == NULL)
        return;

    std::string reason;
    if (cmd.hasTrailing)
        reason = cmd.message;
    else if (cmd.params.size() > 1)
    {
        reason = cmd.params[1];
        for (std::size_t i = 2; i < cmd.params.size(); ++i)
            reason += " " + cmd.params[i];
    }

    const std::string &channelParam = cmd.params[0];

    
    std::vector<std::string> targets;
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

    for (std::size_t i = 0; i < targets.size(); ++i)
    {
        const std::string &targetName = targets[i];
        Channel *channel = requireChannel(fd, targetName);
        if (channel == NULL)
            continue;
        if (!requireChannelMember(fd, *channel))
            continue;

        std::string partMsg = IRC::ResponseBuilder::clientMessage(
            user->getFullIdentifier(), "PART", channel->getName(), reason);

        broadcastToChannel(*channel, partMsg);

        
        removeFromChannel(fd, *channel);
    }
}
