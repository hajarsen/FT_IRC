#include "bonus.hpp"

bool Bot::isTarget(const std::string &target)
{
    if (target.size() != 3)
        return false;
    
    static const char expected[] = "bot";
    for (std::size_t i = 0; i < 3; ++i)
    {
        unsigned char c = static_cast<unsigned char>(target[i]);
        if (c >= 'A' && c <= 'Z')
            c = static_cast<unsigned char>(c - 'A' + 'a');
        else if (c == '{')
            c = '[';
        else if (c == '}')
            c = ']';
        else if (c == '|')
            c = '\\';
        else if (c == '~')
            c = '^';
        if (c != static_cast<unsigned char>(expected[i]))
            return false;
    }
    return true;
}

std::string Bot::reply(const std::string &nickname,
                       const std::string &message)
{
    if (message == "!hello")
        return "Hello " + nickname + "!";
    if (message == "!help")
        return "Commands: !hello !help";
    return "Unknown command. Try !help";
}