#pragma once

#include "LocationConfig.hpp"
#include <string>
#include <vector>
#include <map>

class Config
{
private:
    int _port;
    std::string _serverName;
    // std::string _host;
    std::string _root;
    std::string _index;
    size_t _clientMaxBodySize;
    std::map<int, std::string> _errorPages;
    std::vector<LocationConfig> _locations;

public:
    Config();
    void setPort(const std::string &port);
    void setServerName(const std::string &name);
    void setRoot(const std::string &root);
    void setIndex(const std::string &index);
    void setClientMaxBodySize(const std::string &size);
    void addErrorPage(const std::string &code, const std::string &path);
    void addLocation(const LocationConfig &loc);

    int getPort() const;  
    const std::string &getServerName() const;
    // const std::string &getHost() const;
    const std::string &getRoot() const;
    const std::string &getIndex() const;
    size_t getClientMaxBodySize() const;
    const std::map<int, std::string> &getErrorPages() const;
    const std::vector<LocationConfig> &getLocations() const;

    const LocationConfig *matchLocation(const std::string &uri) const;
    void print() const; 
    std::string stripSemicolon(const std::string &s);
    std::string intToStr(int n);
};