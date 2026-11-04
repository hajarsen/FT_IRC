#include "CommandParser.hpp"

#include <cctype>




static const std::string::size_type MAX_NICK_LEN = 30;
static const std::string::size_type MAX_CHANNEL_LEN = 50;

static bool isNickSpecial(unsigned char c)
{
    return c == '[' || c == ']' || c == '\\' ||
           c == '`' || c == '_' || c == '^' ||
           c == '{' || c == '|' || c == '}';
}

Command CommandParser::parse(const std::string &line)
{
    Command cmd;
    std::string::size_type end = line.size();

    if (end == 0)
        return cmd;

    for (std::string::size_type i = 0; i < end; ++i)
    {
        unsigned char c = static_cast<unsigned char>(line[i]);
        if (c == '\x01')
            continue;
        if (c == '\0' || c == '\r' || c == '\n' || std::iscntrl(c))
            return cmd;
    }

    std::string::size_type start = 0;
    if (line[0] == ':')
    {
        std::string::size_type pos = line.find(' ');

        if (pos == std::string::npos || pos >= end)
            return cmd;

        cmd.prefix = line.substr(1, pos - 1);
        start = pos + 1;

        if (start >= end)
            return cmd;
    }

    std::string::size_type trailingPos = line.find(" :", start);

    if (trailingPos != std::string::npos && trailingPos < end)
    {
        cmd.message = line.substr(trailingPos + 2, end - trailingPos - 2);
        cmd.hasTrailing = true;
        end = trailingPos;
    }

    while (start < end && line[start] == ' ')
        ++start;

    if (start == end)
        return cmd;

    std::string::size_type nameEnd = start;
    while (nameEnd < end && line[nameEnd] != ' ')
        ++nameEnd;

    cmd.name = line.substr(start, nameEnd - start);
    for (std::string::size_type i = 0; i < cmd.name.size(); ++i)
        cmd.name[i] = static_cast<char>(std::toupper(static_cast<unsigned char>(cmd.name[i])));

    start = nameEnd;
    while (start < end)
    {
        while (start < end && line[start] == ' ')
            ++start;

        if (start == end)
            break;

        std::string::size_type paramEnd = start;
        while (paramEnd < end && line[paramEnd] != ' ')
            ++paramEnd;

        cmd.params.push_back(line.substr(start, paramEnd - start));
        start = paramEnd;
    }

    return cmd;
}

bool CommandParser::isValidNickname(const std::string &nick)
{
    if (nick.empty() || nick.size() > MAX_NICK_LEN)
        return false;

    unsigned char first = static_cast<unsigned char>(nick[0]);

    if (!std::isalpha(first) && !isNickSpecial(first))
        return false;

    for (std::string::size_type i = 1; i < nick.size(); ++i)
    {
        unsigned char c = static_cast<unsigned char>(nick[i]);

        if (!std::isalnum(c) && !isNickSpecial(c) && c != '-')
            return false;
    }

    return true;
}

bool CommandParser::isValidChannelName(const std::string &name)
{
    if (name.size() < 2 || name.size() > MAX_CHANNEL_LEN)
        return false;

    if (name[0] != '#' && name[0] != '&')
        return false;

    for (std::string::size_type i = 1; i < name.size(); ++i)
    {
        unsigned char c = static_cast<unsigned char>(name[i]);

        if (c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == ',')
            return false;

        if (std::iscntrl(c))
            return false;
    }

    return true;
}
