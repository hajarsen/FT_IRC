#include "Response.hpp"

namespace IRC
{
    const std::string &serverName()
    {
        static const std::string name = "ft_irc";
        return name;
    }

    static std::string buildNumeric(int code, const std::string &nick, const std::string &msg)
    {
        char codeText[4];
        codeText[0] = static_cast<char>('0' + (code / 100) % 10);
        codeText[1] = static_cast<char>('0' + (code / 10) % 10);
        codeText[2] = static_cast<char>('0' + code % 10);
        codeText[3] = '\0';

        std::string response;
        response.reserve(serverName().size() + nick.size() + msg.size() + 10);
        response += ":" + serverName() + " " + codeText + " " + nick;
        if (!msg.empty())
            response += " " + msg;
        response += "\r\n";
        return response;
    }

    std::string ResponseBuilder::numericResponse(int code, const std::string &nick, const std::string &msg)
    {
        return buildNumeric(code, nick, msg);
    }

    std::string ResponseBuilder::clientMessage(const std::string &fromNick, const std::string &command,
                                               const std::string &params, const std::string &msg,
                                               bool includeEmptyMessage)
    {
        std::string response;
        response.reserve(fromNick.size() + command.size() + params.size() + msg.size() + 7);
        response += ":" + fromNick + " " + command;
        if (!params.empty())
            response += " " + params;
        if (!msg.empty() || includeEmptyMessage)
            response += " :" + msg;
        response += "\r\n";
        return response;
    }
}
