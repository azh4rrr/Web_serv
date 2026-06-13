#pragma once
#include "../config/Config.hpp"
#include <sys/socket.h>

class Server {
private:
    int _fd;
    Config _config;

public:
    Server() : _fd(-1) {}
    Server(const Config &config);
    
    int getFd() const { return _fd; }
    const Config& getConfig() const { return _config; }
};



