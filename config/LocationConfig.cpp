
#include "LocationConfig.hpp"
#include <iostream>
#include <sstream>

const std::string              &LocationConfig::getPath()         const { return _path; }
const std::string              &LocationConfig::getRoot()         const { return _root; }
const std::string              &LocationConfig::getIndex()        const { return _index; }
bool                            LocationConfig::getAutoindex()    const { return _autoindex; }
const std::vector<std::string> &LocationConfig::getMethods()      const { return _methods; }
const std::string              &LocationConfig::getCgiExtension() const { return _cgiExtension; }
const std::string              &LocationConfig::getCgiPath()      const { return _cgiPath; }
const std::string              &LocationConfig::getReturn()       const { return _return; }


LocationConfig::LocationConfig() : _autoindex(false) {}

void LocationConfig::setPath(const std::string &path) { _path = path; }

void LocationConfig::setRoot(const std::string &root)
{
    _root = stripSemicolon(root);
    if (_root.empty())
        throw std::runtime_error("Location: root value is empty");
}

void LocationConfig::setIndex(const std::string &index)
{
    _index = stripSemicolon(index);
}

void LocationConfig::setAutoindex(const std::string &value)
{
    std::string v = stripSemicolon(value);
    if (v == "on")
        _autoindex = true;
    else if (v == "off")
        _autoindex = false;
    else
        throw std::runtime_error("Location [" + _path + "]: autoindex must be 'on' or 'off', got '" + v + "'");
}

void LocationConfig::setMethods(const std::vector<std::string> &methods)
{
    const std::string valid[] = {"GET", "POST", "DELETE"};
    const int validCount = 3;
    std::string m;
    _methods.clear();

    size_t i = 0;
    while (i < methods.size() - 1)
    {
        m = methods[i];
        bool found = false;
        for (int j = 0; j < validCount; ++j)
            if (m == valid[j])
            {
                found = true;
                break;
            }
        if (!found)
            throw std::runtime_error("Location [" + _path + "]: unknown method '" + m + "'");
        _methods.push_back(m);
        i++;
    }
    m = stripSemicolon(methods[i]);
    _methods.push_back(m);
    if (_methods.empty())
        throw std::runtime_error("Location [" + _path + "]: 'methods' directive has no values");
}

void LocationConfig::setCgiExtension(const std::string &ext)
{
    _cgiExtension = stripSemicolon(ext);
    if (!_cgiExtension.empty() && _cgiExtension[0] != '.')
        throw std::runtime_error("Location [" + _path + "]: cgi_extension must start with '.', got '" + _cgiExtension + "'");
}

void LocationConfig::setCgiPath(const std::string &path)
{
    _cgiPath = stripSemicolon(path);
    if (_cgiPath.empty())
        throw std::runtime_error("Location [" + _path + "]: cgi_path value is empty");
}

void LocationConfig::setReturn(const std::string &url)
{
    _return = stripSemicolon(url);
}

std::string LocationConfig::stripSemicolon(const std::string &s)
{
    if (!s.empty() && s[s.size() - 1] == ';')
        return s.substr(0, s.size() - 1);
    else
        throw std::runtime_error("Value must end with ';', got '" + s + "'");
}

std::string LocationConfig::intToStr(int n) { 
    std::ostringstream ss; 
    ss << n; 
    return ss.str(); 
}

bool LocationConfig::isMethodAllowed(const std::string &method) const
{
    for (size_t i = 0; i < _methods.size(); ++i)
        if (_methods[i] == method) return true;
    return false;
}

void LocationConfig::print() const
{
    std::cout << "  location " << _path << " {\n";
    if (!_root.empty())         std::cout << "    root          " << _root << "\n";
    if (!_index.empty())        std::cout << "    index         " << _index << "\n";
    std::cout                             << "    autoindex     " << (_autoindex ? "on" : "off") << "\n";
    if (!_methods.empty()) {
        std::cout << "    methods       ";
        for (size_t i = 0; i < _methods.size(); ++i)
            std::cout << _methods[i] << " ";
        std::cout << "\n";
    }
    if (!_cgiExtension.empty()) std::cout << "    cgi_extension " << _cgiExtension << "\n";
    if (!_cgiPath.empty())      std::cout << "    cgi_path      " << _cgiPath << "\n";
    if (!_return.empty())       std::cout << "    return        " << _return << "\n";
    std::cout << "  }\n";
}

