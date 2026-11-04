#ifndef USER_HPP
#define USER_HPP

#include <string>

class User
{
private:
    int fd;
    std::string nickname;
    std::string username;
    std::string realname;
    bool isAuthenticated;
    bool isRegistered;

public:
    User() : fd(-1), isAuthenticated(false), isRegistered(false) {}
    User(int clientFd) : fd(clientFd), isAuthenticated(false), isRegistered(false) {}

    int getFd() const { return fd; }
    const std::string &getNickname() const { return nickname; }
    const std::string &getUsername() const { return username; }
    const std::string &getRealname() const { return realname; }
    bool getIsAuthenticated() const { return isAuthenticated; }
    bool getIsRegistered() const { return isRegistered; }

    void setNickname(const std::string &nick) { nickname = nick; }
    void setUsername(const std::string &user) { username = user; }
    void setRealname(const std::string &real) { realname = real; }
    void setIsAuthenticated(bool auth) { isAuthenticated = auth; }
    void setIsRegistered(bool reg) { isRegistered = reg; }

    std::string getFullIdentifier() const
    {
        std::string identifier;
        identifier.reserve(nickname.size() + username.size() + 11);
        identifier += nickname + "!" + username + "@localhost";
        return identifier;
    }
};

#endif

