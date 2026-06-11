#pragma once 
#include "config/ParssingConf.hpp"
#include "server/server.hpp"
#include "server/Server.hpp" 
#include <vector>
#include <poll.h>

class Webserv
{
private:
    std::vector<Server> _servers;
    std::vector<pollfd> _pollfds;

public:
    Webserv(){};

    // bool is_server(int fd) const {
    //     for (size_t i = 0; i < _servers.size(); ++i) {
    //         if (_servers[i].getFd() == fd) {
    //             return true;
    //         }
    //     }
    //     return false;
    // }

    void setupServers(const std::string &configFile);
    
};


void Webserv::setupServers(const std::string &configFile)
{
    ParssingConf parser;
    parser.parseConfig(configFile);
    const std::vector<Config> &configs = parser.getConfigs();

    for (size_t i = 0; i < configs.size(); ++i)
    {
        Server server(configs[i]);
        
        // and add them to the _servers vector. For example:
        // Server server(configs[i]);
        // _servers.push_back(server);
    }
}