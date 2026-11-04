#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <string>

namespace IRC
{
    const int RPL_WELCOME = 1;
    const int RPL_YOURHOST = 2;
    const int RPL_CREATED = 3;
    const int RPL_MYINFO = 4;
    const int RPL_CHANNELMODEIS = 324;
    const int RPL_NOTOPIC = 331;
    const int RPL_TOPIC = 332;
    const int RPL_INVITING = 341;
    const int RPL_NAMREPLY = 353;
    const int RPL_ENDOFNAMES = 366;
    const int ERR_NOSUCHNICK = 401;
    const int ERR_NOSUCHNICKATTEMPT = 401;
    const int ERR_NOSUCHCHANNEL = 403;
    const int ERR_CANNOTSENDTOCHAN = 404;
    const int ERR_NORECIPIENT = 411;
    const int ERR_NOTEXTTOSEND = 412;
    const int ERR_UNKNOWNCOMMAND = 421;
    const int ERR_NONICKNAMEGIVEN = 431;
    const int ERR_ERRONEUSNICKNAME = 432;
    const int ERR_NICKNAMEINUSE = 433;
    const int ERR_USERNOTINCHANNEL = 441;
    const int ERR_NOTONCHANNEL = 442;
    const int ERR_USERONCHANNEL = 443;
    const int ERR_NOTREGISTERED = 451;
    const int ERR_NEEDMOREPARAMS = 461;
    const int ERR_ALREADYREGISTERED = 462;
    const int ERR_PASSWDMISMATCH = 464;
    const int ERR_CHANNELISFULL = 471;
    const int ERR_UNKNOWNMODE = 472;
    const int ERR_INVITEONLYCHAN = 473;
    const int ERR_BADCHANNELKEY = 475;
    const int ERR_CHANOPRIVSNEEDED = 482;

    const std::string &serverName();

    class ResponseBuilder
    {
    public:
        static std::string numericResponse(int code, const std::string &nick, const std::string &msg);
        static std::string clientMessage(const std::string &fromNick, const std::string &command,
                                         const std::string &params, const std::string &msg,
                                         bool includeEmptyMessage = false);
    };
}

#endif
