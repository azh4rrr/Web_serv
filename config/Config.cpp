#include "Config.hpp"
#include <iostream>
#include <sstream>

int                                Config::getPort()              const { return _port; }
const std::string                 &Config::getServerName()        const { return _serverName; }
// const std::string                 &Config::getHost()              const { return _host; }
const std::string                 &Config::getRoot()              const { return _root; }
const std::string                 &Config::getIndex()             const { return _index; }
size_t                             Config::getClientMaxBodySize() const { return _clientMaxBodySize; }
const std::map<int,std::string>   &Config::getErrorPages()        const { return _errorPages; }
const std::vector<LocationConfig> &Config::getLocations()         const { return _locations; }

Config::Config()
    : _port(0),
      _clientMaxBodySize(1000000)   // default 1 MB
{}

void Config::setPort(const std::string &port)
{
    std::string p = stripSemicolon(port);
    for (size_t i = 0; i < p.size(); ++i)
        if (!isdigit(p[i]))
            throw std::runtime_error("Server: port must be a number, got '" + p + "'");
    int val = std::atoi(p.c_str());
    if (val < 1 || val > 65535)
        throw std::runtime_error("Server: port out of range (1-65535), got " + p);
    _port = val;
}

void Config::setServerName(const std::string &name)
{
    _serverName = stripSemicolon(name);
    if (_serverName.empty())
        throw std::runtime_error("Server: server_name value is empty");
}

void Config::setRoot(const std::string &root)
{
    _root = stripSemicolon(root);
    if (_root.empty())
        throw std::runtime_error("Server: root value is empty");
}

void Config::setIndex(const std::string &index)
{
    _index = stripSemicolon(index);
    if (_index.empty())
        throw std::runtime_error("Server: index value is empty");
}

void Config::setClientMaxBodySize(const std::string &size)
{
    std::string s = stripSemicolon(size);
    for (size_t i = 0; i < s.size(); ++i)
        if (!isdigit(s[i]))
            throw std::runtime_error("Server: client_max_body_size must be a number, got '" + s + "'");

    long long val = std::atoll(s.c_str());
    if (val <= 0)
        throw std::runtime_error("Server: client_max_body_size must be > 0");

    _clientMaxBodySize = static_cast<size_t>(val);
}

void Config::addErrorPage(const std::string &code, const std::string &path)
{
    for (size_t i = 0; i < code.size(); ++i)
        if (!isdigit(code[i]))
            throw std::runtime_error("Server: error_page code must be a number, got '" + code + "'");

    int c = std::atoi(code.c_str());
    if (c < 400 || c > 599)
        throw std::runtime_error("Server: error_page code must be 4xx or 5xx, got " + code);

    std::string p = stripSemicolon(path);
    if (p.empty())
        throw std::runtime_error("Server: error_page path is empty for code " + code);

    _errorPages[c] = p;
}

void Config::addLocation(const LocationConfig &loc)
{
    // no duplicate location paths
    for (size_t i = 0; i < _locations.size(); ++i)
        if (_locations[i].getPath() == loc.getPath())
            throw std::runtime_error("Server port " + intToStr(_port) + ": duplicate location '" + loc.getPath() + "'");
    _locations.push_back(loc);
}

const LocationConfig *Config::matchLocation(const std::string &uri) const
{
    const LocationConfig *best = NULL;
    size_t bestLen = 0;

    for (size_t i = 0; i < _locations.size(); ++i)
    {
        const std::string &locPath = _locations[i].getPath();
        if (uri.compare(0, locPath.size(), locPath) != 0)
            continue;
        if (uri.size() > locPath.size() && uri[locPath.size()] != '/')
            continue;

        if (locPath.size() > bestLen)
        {
            bestLen = locPath.size();
            best = &_locations[i];
        }
    }
    return best;
}

void Config::print() const
{
    std::cout << "server {\n";
    std::cout << "  listen        " << _port << "\n";
    std::cout << "  server_name   " << _serverName << "\n";
    std::cout << "  root          " << _root << "\n";
    std::cout << "  index         " << _index << "\n";
    std::cout << "  max_body      " << _clientMaxBodySize << "\n";
    for (std::map<int,std::string>::const_iterator it = _errorPages.begin();
         it != _errorPages.end(); ++it)
        std::cout << "  error_page    " << it->first << " " << it->second << "\n";
    for (size_t i = 0; i < _locations.size(); ++i)
        _locations[i].print();
    std::cout << "}\n";
}

std::string Config::stripSemicolon(const std::string &s)
{
    if (!s.empty() && s[s.size() - 1] == ';')
        return s.substr(0, s.size() - 1);
    else
        throw std::runtime_error("Value must end with ';', got '" + s + "'");
}

std::string Config::intToStr(int n) { 
    std::ostringstream ss; 
    ss << n; 
    return ss.str(); 
}
