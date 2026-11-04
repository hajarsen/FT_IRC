#ifndef COMMANDPARSER_HPP
#define COMMANDPARSER_HPP

#include <string>
#include <vector>

struct Command
{
    std::string prefix;
    std::string name;
    std::vector<std::string> params;
    std::string message;
    bool hasTrailing;

    Command() : hasTrailing(false) {}
    bool isValid() const { return !name.empty(); }
};




class CommandParser
{
public:
    static Command parse(const std::string &line);
    static bool isValidNickname(const std::string &nick);
    static bool isValidChannelName(const std::string &name);

private:
    CommandParser();
};

#endif
