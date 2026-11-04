#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <vector>
#include <cstddef>

class Client
{
public:
    static const std::size_t MAX_OUT_BUFFER = 64 * 1024;

private:
    int         _fd;
    std::string _inBuffer;
    std::string _outBuffer;
    bool        _closing;

public:
    Client(int fd);
    ~Client();
    int getFd() const;

    
    void feed(const char *data, std::size_t len);
    std::vector<std::string> extractLines();

    
    void enqueue(const std::string &data);
    bool hasOutput() const;
    std::size_t getOutBufferSize() const;
    void closeAfterFlush();
    bool isClosing() const;

    enum FlushResult { FlushOk, FlushWouldBlock, FlushError };
    FlushResult flush();
};

#endif
