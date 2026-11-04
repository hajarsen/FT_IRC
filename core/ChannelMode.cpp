#include "ChannelMode.hpp"

#include <sstream>

ChannelMode::ChannelMode()
    : inviteOnly(false), topicOpOnly(false), keyRequired(false), key(""), userLimitSet(false), userLimit(0) {}

std::string ChannelMode::getModeString() const
{
    std::string modes = "+";
    if (inviteOnly) modes += "i";
    if (topicOpOnly) modes += "t";
    if (keyRequired) modes += "k";
    if (userLimitSet) modes += "l";
    return modes;
}

std::string ChannelMode::getModeArgs() const
{
    std::string args;
    if (keyRequired)
        args += " " + key;
    if (userLimitSet)
    {
        std::ostringstream oss;
        oss << userLimit;
        args += " " + oss.str();
    }
    return args;
}
