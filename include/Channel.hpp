#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <set>
#include "ChannelMode.hpp"

class Channel
{
private:
    std::string name;
    std::string topic;
    std::set<int> members;
    std::set<int> operators;
    std::set<int> invited;
    ChannelMode modes;

public:
    Channel() {}
    Channel(const std::string &channelName) : name(channelName) {}

    const std::string &getName() const { return name; }
    const std::string &getTopic() const { return topic; }
    const std::set<int> &getMembers() const { return members; }
    const std::set<int> &getOperators() const { return operators; }
    ChannelMode &getModes() { return modes; }
    const ChannelMode &getModes() const { return modes; }

    void setTopic(const std::string &t) { topic = t; }

    void addMember(int fd) { members.insert(fd); }
    void removeMember(int fd) { members.erase(fd); operators.erase(fd); invited.erase(fd); }
    bool hasMember(int fd) const { return members.find(fd) != members.end(); }
    int getMemberCount() const { return static_cast<int>(members.size()); }

    void addOperator(int fd) { operators.insert(fd); }
    void removeOperator(int fd) { operators.erase(fd); }
    bool isOperator(int fd) const { return operators.find(fd) != operators.end(); }

    void addInvited(int fd) { invited.insert(fd); }
    void removeInvited(int fd) { invited.erase(fd); }
    bool isInvited(int fd) const { return invited.find(fd) != invited.end(); }

    bool isEmpty() const { return members.empty(); }
};

#endif
