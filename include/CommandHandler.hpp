#ifndef COMMANDHANDLER_HPP
#define COMMANDHANDLER_HPP

#include <string>
#include <map>
#include <set>
#include <vector>
#include "CommandParser.hpp"
#include "User.hpp"
#include "Channel.hpp"
#include "Response.hpp"

class CommandHandler
{
private:
    std::map<int, User> users;
    std::map<std::string, Channel> channels;
    std::map<int, std::string> _responses;
    std::string _serverPassword;

    void handlePASS(int fd, const Command &cmd);
    void handleNICK(int fd, const Command &cmd);
    void handleUSER(int fd, const Command &cmd);
    void handleJOIN(int fd, const Command &cmd);
    void handlePRIVMSG(int fd, const Command &cmd);
    void handleMODE(int fd, const Command &cmd);
    void handleTOPIC(int fd, const Command &cmd);
    void handleINVITE(int fd, const Command &cmd);
    void handleKICK(int fd, const Command &cmd);
    void handlePART(int fd, const Command &cmd);
    void handleQUIT(int fd, const Command &cmd);
    void handlePING(int fd, const Command &cmd);
    void handlePONG(int fd, const Command &cmd);
    void handleNOTICE(int fd, const Command &cmd);
    

    Channel *findChannel(const std::string &name);
    const Channel *findChannel(const std::string &name) const;
    Channel *requireChannel(int fd, const std::string &name);
    bool requireChannelMember(int fd, Channel &channel);
    bool requireChannelOperator(int fd, Channel &channel);
    void needMoreParams(int fd, const std::string &command);
    
    
    
    
    
    void addToChannel(int fd, Channel &channel);
    void removeFromChannel(int fd, Channel &channel);
    void removeFromAllChannels(int fd);
    void eraseChannelIfEmpty(const std::string &lowerName);
    void joinChannel(int fd, const std::string &name, const std::string &key);
    void sendMessageToTarget(int fd, const Command &cmd, bool isNotice = false);
    std::set<int> _pendingDisconnect;
     
    bool nextModeArgument(const Command &cmd, std::size_t &index, std::string &out);
    Channel *findModeChannel(int fd, const Command &cmd);
    bool applyKeyLimitMode(int fd, Channel &channel, const Command &cmd, bool adding,
                           char mode, std::size_t &argumentIndex,
                           std::vector<std::string> &modeArguments);
    bool applyOperatorMode(int fd, Channel &channel, const Command &cmd, bool adding,
                           std::size_t &argumentIndex, std::vector<std::string> &modeArguments);
    void broadcastModeChange(int fd, Channel &channel, const std::string &appliedModes,
                             const std::vector<std::string> &modeArguments);

     
    User *getUser(int fd);
    const User *getUser(int fd) const;
    User *getUserByNick(const std::string &nick);
    User *getOrCreateUser(int fd);
    void sendWelcome(int fd);
    void tryRegister(int fd);
    void sendNumeric(int fd, int code, const std::string &text);
    void sendToClient(int fd, const std::string &data);
    void broadcastToChannel(const Channel &channel, const std::string &message,
                            int senderFd = -1);
    static std::string toLower(const std::string &s);

public:
    CommandHandler();
    virtual ~CommandHandler();

    void setServerPassword(const std::string &password);

    void processLine(int fd, const std::string &line);
    void clientConnected(int fd);
    void clientDisconnected(int fd);
    void resetSessions() throw();
    std::vector<int> getMembersOfChannel(const std::string &channelName) const;

    struct DrainEffects
    {
        std::map<int, std::string> replies;
        std::vector<int> disconnects;
    };
    DrainEffects drain();
};


#endif
