#include "Client.hpp"

#include <unistd.h>
#include <sys/socket.h>
#include <errno.h>

const std::size_t Client::MAX_OUT_BUFFER;

Client::Client(int fd) : _fd(fd), _closing(false)
{
}

Client::~Client()
{
    if (_fd != -1)
        close(_fd);
}

int Client::getFd() const
{
    return _fd;
}

void Client::feed(const char *data, std::size_t len)
{
    _inBuffer.append(data, len);
}

std::vector<std::string> Client::extractLines()
{
    std::vector<std::string> lines;
    const std::size_t MAX_LINE = 512;

    if (_inBuffer.size() > MAX_LINE && _inBuffer.find('\n') == std::string::npos)
    {
        _inBuffer.clear();
        return lines;
    }

    std::size_t pos;
    while ((pos = _inBuffer.find('\n')) != std::string::npos)
    {
        std::string raw = _inBuffer.substr(0, pos + 1);
        _inBuffer.erase(0, pos + 1);

        if (raw.size() > MAX_LINE)
        {
            
            
            
            
            
            
            const char *cmds[] = {"PASS","NICK","USER","JOIN","PRIVMSG","NOTICE","MODE","TOPIC","INVITE","KICK","PART","QUIT","PING","PONG"};
            const std::size_t cmdCount = sizeof(cmds) / sizeof(cmds[0]);
            std::size_t rescuePos = std::string::npos;
            for (std::size_t p = 0; p < raw.size(); ++p)
            {
                if (raw.size() - p > MAX_LINE)
                    continue;
                for (std::size_t ci = 0; ci < cmdCount; ++ci)
                {
                    const std::string cmd = cmds[ci];
                    if (p + cmd.size() > raw.size())
                        continue;
                    
                    bool match = true;
                    for (std::size_t k = 0; k < cmd.size(); ++k)
                    {
                        unsigned char a = static_cast<unsigned char>(raw[p + k]);
                        unsigned char b = static_cast<unsigned char>(cmd[k]);
                        if (std::toupper(a) != b) { match = false; break; }
                    }
                    if (!match) continue;
                    if (p + cmd.size() < raw.size())
                    {
                        char after = raw[p + cmd.size()];
                        if (after != ' ' && after != '\r' && after != '\n' && after != ':')
                            continue;
                    }
                    rescuePos = p;
                    break;
                }
                if (rescuePos != std::string::npos)
                    break;
            }
            if (rescuePos != std::string::npos)
            {
                std::string rescued = raw.substr(rescuePos);
                std::string line = rescued;
                if (!line.empty() && line[line.size() - 1] == '\n')
                    line.erase(line.size() - 1);
                if (!line.empty() && line[line.size() - 1] == '\r')
                    line.erase(line.size() - 1);
                lines.push_back(line);
            }
            continue;
        }

        std::string line = raw;
        if (!line.empty() && line[line.size() - 1] == '\n')
            line.erase(line.size() - 1);
        if (!line.empty() && line[line.size() - 1] == '\r')
            line.erase(line.size() - 1);
        lines.push_back(line);
    }
    return lines;
}

void Client::enqueue(const std::string &data)
{
    _outBuffer += data;
}

bool Client::hasOutput() const
{
    return !_outBuffer.empty();
}

std::size_t Client::getOutBufferSize() const
{
    return _outBuffer.size();
}

void Client::closeAfterFlush()
{
    _closing = true;
}

bool Client::isClosing() const
{
    return _closing;
}

Client::FlushResult Client::flush()
{
    if (_outBuffer.empty())
        return FlushOk;

    ssize_t bytesSent = send(_fd, _outBuffer.c_str(), _outBuffer.size(), 0);
    if (bytesSent < 0)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
            return FlushWouldBlock;
        return FlushError;
    }
    if (bytesSent == 0)
        return FlushWouldBlock;
    _outBuffer.erase(0, static_cast<std::size_t>(bytesSent));
    return _outBuffer.empty() ? FlushOk : FlushWouldBlock;
}
