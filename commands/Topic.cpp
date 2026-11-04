#include "CommandHandler.hpp"

void CommandHandler::handleTOPIC(int fd, const Command &cmd)
{
    if (cmd.params.empty()) return needMoreParams(fd, cmd.name);
    Channel *channel = requireChannel(fd, cmd.params[0]);
    if (!channel || !requireChannelMember(fd, *channel)) return;
    if (!cmd.hasTrailing)
    {
        const bool hasTopic = !channel->getTopic().empty();
        return sendNumeric(fd, hasTopic ? IRC::RPL_TOPIC : IRC::RPL_NOTOPIC,
            channel->getName() + (hasTopic ? " :" + channel->getTopic() : " :No topic is set"));
    }
    if (channel->getModes().isTopicOpOnly() && !requireChannelOperator(fd, *channel)) return;
    channel->setTopic(cmd.message);
    broadcastToChannel(*channel, IRC::ResponseBuilder::clientMessage(
        getUser(fd)->getFullIdentifier(), "TOPIC", channel->getName(), channel->getTopic(), true));
}
