#ifndef BOT_HPP
#define BOT_HPP

#include <string>

class Bot
{
public:
    static bool isTarget(const std::string &target);
    static std::string reply(const std::string &nickname,
                             const std::string &message);
};

#endif