#include "CommandHandler.hpp"

void CommandHandler::handleQUIT(int fd, const Command &cmd)
{
    User *user = getUser(fd);
    std::string reason;

    if (cmd.hasTrailing)
        reason = cmd.message;
    else if (!cmd.params.empty())
    {
        reason = cmd.params[0];
        for (std::size_t i = 1; i < cmd.params.size(); ++i)
            reason += " " + cmd.params[i];
    }

    
    if (user == NULL)
    {
        _pendingDisconnect.insert(fd);
        return;
    }

    std::string quitMsg = IRC::ResponseBuilder::clientMessage(
        user->getFullIdentifier(), "QUIT", "", reason);

    
    
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

    
    
    
    
    {
        std::string closingReason = reason.empty() ? "Client Quit" : reason;
        sendToClient(fd, "ERROR :Closing Link: " + user->getNickname() + " (" + closingReason + ")\r\n");
    }

    
    removeFromAllChannels(fd);
    _pendingDisconnect.insert(fd);
}
