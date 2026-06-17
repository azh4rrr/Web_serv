#include "LocationConfig.hpp"

const std::string &LocationConfig::getPath() const { return _path; }

LocationConfig::LocationConfig() : _path("") {}

void LocationConfig::setPath(const std::string &path) { _path = path; }

#include <iostream>
LocationConfig::~LocationConfig() {
    // _path.clear();
    // _root.clear();
    // _index.clear();
    // _autoindex = false;
    // _clientMaxBodySize.clear();
    // _errorPages.clear();
    // _methods.clear();
    // _cgiExtension.clear();
    // _cgiPath.clear();
    // _return.clear();
}

void LocationConfig::print() const {
    std::cout << "Location Path: " << _path << std::endl;
    std::cout << "Root: " << _root << std::endl;
    std::cout << "Index: " << _index << std::endl;
    std::cout << "Autoindex: " << (_autoindex ? "on" : "off") << std::endl;
    std::cout << "Client Max Body Size: " << _clientMaxBodySize << std::endl;
    std::cout << "Methods: ";
    for (size_t i = 0; i < _methods.size(); ++i)
        std::cout << _methods[i] << (i < _methods.size() - 1 ? ", " : "");
    std::cout << std::endl;
    std::cout << "CGI Extension: " << _cgiExtension << std::endl;
    std::cout << "CGI Path: " << _cgiPath << std::endl;
    std::cout << "Return: " << _return << std::endl;
}

bool LocationConfig::isMethodAllowed(int method) const
{
    if (_methods.empty()) {
        return true; // If no methods specified, allow all methods
    }
    std::string methods[] = {"GET", "POST", "DELETE"};
    for (size_t i = 0; i < _methods.size(); ++i)
        if (_methods[i] == methods[method]) return true;
    return false;
}

// #include <iostream>
// void LocationConfig::PrintLocationConfig() const
// {
//     std::cout << "Location Path: " << _path << std::endl;
//     std::cout << "Root: " << _root << std::endl;
//     std::cout << "Index: " << _index << std::endl;
//     std::cout << "Autoindex: " << (_autoindex ? "on" : "off") << std::endl;
//     std::cout << "Client Max Body Size: " << _clientMaxBodySize << std::endl;
//     std::cout << "Methods: ";
//     for (size_t i = 0; i < _methods.size(); ++i)
//         std::cout << _methods[i] << (i < _methods.size() - 1 ? ", " : "");
//     std::cout << std::endl;
//     std::cout << "CGI Extension: " << _cgiExtension << std::endl;
//     std::cout << "CGI Path: " << _cgiPath << std::endl;
//     std::cout << "Return: " << _return << std::endl;
// }
