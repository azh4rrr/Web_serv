#pragma once
#include "../config/Config.hpp"
#include <sys/socket.h>

class Server {
private:
    int _fd;
    Config _config;

public:
    Server() : _fd(-1) {}
    Server(const Config &config) : _config(config), _fd(-1) {}
    void createSocket() {
        
    }

};



Server::Server(const Config &config) : _config(config) {
    _fd = socket(AF_INET, SOCK_STREAM, 0);
    if (_fd < 0) {
        throw std::runtime_error("Failed to create socket");
    }
    int opt = 1;
    if (setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR , &opt, sizeof(opt)) < 0) {
        throw std::runtime_error("Failed to set socket options");
    }

}

