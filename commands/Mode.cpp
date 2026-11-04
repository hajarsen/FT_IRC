#include "CommandHandler.hpp"

#include <climits>

static bool parseLimit(const std::string &text, int &limit)
{
    int value = 0;

    if (text.empty()) return false;
    for (std::string::size_type i = 0; i < text.size(); ++i)
    {
        if (text[i] < '0' || text[i] > '9' ||
            value > (INT_MAX - (text[i] - '0')) / 10) return false;
        value = value * 10 + (text[i] - '0');
    }
    if (value == 0) return false;
    limit = value;
    return true;
}

Channel *CommandHandler::findModeChannel(int fd, const Command &cmd)
{
    if (cmd.params.empty())
    {
        needMoreParams(fd, cmd.name);
        return NULL;
    }
    const std::string &target = cmd.params[0];
    if (target.empty() || (target[0] != '#' && target[0] != '&'))
    {
        sendNumeric(fd, IRC::ERR_NOSUCHCHANNEL, target + " :No such channel");
        return NULL;
    }
    return requireChannel(fd, target);
}

bool CommandHandler::nextModeArgument(const Command &cmd, std::size_t &index,
                                      std::string &out)
{
    if (index < cmd.params.size()) out = cmd.params[index++];
    else if (index == cmd.params.size() && cmd.hasTrailing)
    {
        out = cmd.message;
        ++index;
    }
    return !out.empty();
}

bool CommandHandler::applyKeyLimitMode(int fd, Channel &channel, const Command &cmd,
                                       bool adding, char mode, std::size_t &argumentIndex,
                                       std::vector<std::string> &modeArguments)
{
    if (!adding)
    {
        if (mode == 'k') channel.getModes().clearKey();
        else channel.getModes().clearUserLimit();
        return true;
    }
    std::string argument;
    if (!nextModeArgument(cmd, argumentIndex, argument))
    {
        needMoreParams(fd, cmd.name);
        return false;
    }
    if (mode == 'k') channel.getModes().setKey(argument);
    else
    {
        int limit;
        if (!parseLimit(argument, limit))
        {
            sendNumeric(fd, IRC::ERR_NEEDMOREPARAMS, "MODE :Invalid channel limit");
            return false;
        }
        channel.getModes().setUserLimit(limit);
    }
    modeArguments.push_back(argument);
    return true;
}

bool CommandHandler::applyOperatorMode(int fd, Channel &channel, const Command &cmd,
                                       bool adding, std::size_t &argumentIndex,
                                       std::vector<std::string> &modeArguments)
{
    std::string nickname;
    if (!nextModeArgument(cmd, argumentIndex, nickname))
    {
        needMoreParams(fd, cmd.name);
        return false;
    }
    User *target = getUserByNick(nickname);
    if (target == NULL)
    {
        sendNumeric(fd, IRC::ERR_NOSUCHNICKATTEMPT, nickname + " :No such nick");
        return false;
    }
    if (!channel.hasMember(target->getFd()))
    {
        sendNumeric(fd, IRC::ERR_USERNOTINCHANNEL,
                    nickname + " " + channel.getName() + " :They aren't on that channel");
        return false;
    }
    if (adding) channel.addOperator(target->getFd());
    else channel.removeOperator(target->getFd());
    modeArguments.push_back(nickname);
    return true;
}

void CommandHandler::broadcastModeChange(int fd, Channel &channel,
                                         const std::string &appliedModes,
                                         const std::vector<std::string> &arguments)
{
    std::string params = channel.getName() + " " + appliedModes;
    for (std::size_t i = 0; i < arguments.size(); ++i)
        params += " " + arguments[i];
    broadcastToChannel(channel, IRC::ResponseBuilder::clientMessage(
        getUser(fd)->getFullIdentifier(), "MODE", params, ""));
}

void CommandHandler::handleMODE(int fd, const Command &cmd)
{
    Channel *channelPtr = findModeChannel(fd, cmd);

    if (channelPtr == NULL)
        return;

    Channel &channel = *channelPtr;

    if (!requireChannelMember(fd, channel)) return;

    if (cmd.params.size() == 1)
    {
        sendNumeric(fd, IRC::RPL_CHANNELMODEIS,
                    channel.getName() + " " +
                    channel.getModes().getModeString() +
                    channel.getModes().getModeArgs());
        return;
    }

    if (!requireChannelOperator(fd, channel)) return;

    Channel updated(channel);

    const std::string &modeString = cmd.params[1];

    if (modeString.empty())
    {
        sendNumeric(fd, IRC::ERR_UNKNOWNMODE, ":Unknown MODE flag");
        return;
    }

    bool adding = true;
    std::size_t argumentIndex = 2;
    std::string appliedModes;
    std::vector<std::string> modeArguments;

    for (std::size_t i = 0; i < modeString.size(); ++i)
    {
        char current = modeString[i];

        if (current == '+' || current == '-')
        {
            adding = (current == '+');
            continue;
        }

        bool success = false;

        switch (current)
        {
            case 'i':
                updated.getModes().setInviteOnly(adding);
                success = true;
                break;
            case 't':
                updated.getModes().setTopicOpOnly(adding);
                success = true;
                break;
            case 'k':
            case 'l':
                success = applyKeyLimitMode(fd, updated, cmd, adding,
                                            current, argumentIndex, modeArguments);
                break;
            case 'o':
                success = applyOperatorMode(fd, updated, cmd, adding,
                                            argumentIndex, modeArguments);
                break;
            default:
                sendNumeric(fd, IRC::ERR_UNKNOWNMODE,
                            std::string(1, current) + " :Unknown MODE flag");
                continue;
        }

        if (!success)
            return;

        appliedModes += std::string(1, adding ? '+' : '-') + current;
    }

    if (appliedModes.empty())
        return;

    channel = updated;
    broadcastModeChange(fd, channel, appliedModes, modeArguments);
}
