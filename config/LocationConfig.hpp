#pragma once
#include "serverConfig.hpp"

class LocationConfig : public serverConfig 
{
private:
    std::string _path;
    // std::

public:
    LocationConfig();
    virtual ~LocationConfig();

    void setPath(const std::string& path);

    const std::string& getPath() const;

    bool isMethodAllowed(const int method) const;

    void print() const;
};