#ifndef CHANNELMODE_HPP
#define CHANNELMODE_HPP

#include <string>

class ChannelMode
{
private:
    bool inviteOnly;
    bool topicOpOnly;
    bool keyRequired;
    std::string key;
    bool userLimitSet;
    int userLimit;

public:
    ChannelMode();

    bool isInviteOnly() const { return inviteOnly; }
    bool isTopicOpOnly() const { return topicOpOnly; }
    bool isKeyRequired() const { return keyRequired; }
    const std::string &getKey() const { return key; }
    bool isUserLimitSet() const { return userLimitSet; }
    int getUserLimit() const { return userLimit; }

    void setInviteOnly(bool val) { inviteOnly = val; }
    void setTopicOpOnly(bool val) { topicOpOnly = val; }
    void setKey(const std::string &k) { key = k; keyRequired = !k.empty(); }
    void clearKey() { key.clear(); keyRequired = false; }
    void setUserLimit(int limit) { userLimit = limit; userLimitSet = (limit > 0); }
    void clearUserLimit() { userLimitSet = false; userLimit = 0; }

    std::string getModeString() const;
    std::string getModeArgs() const;
};

#endif
