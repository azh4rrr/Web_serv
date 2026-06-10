#pragma once

#include <string>
#include <vector>

class LocationConfig
{
private:
    std::string _path;
    std::string _root;
    std::string _index;
    bool _autoindex;
    std::vector<std::string> _methods;
    std::string _cgiExtension;
    std::string _cgiPath;
    std::string _return;
    
public:
    LocationConfig();
    void setPath(const std::string &path);
    void setRoot(const std::string &root);
    void setIndex(const std::string &index);
    void setAutoindex(const std::string &value);
    void setMethods(const std::vector<std::string> &methods);
    void setCgiExtension(const std::string &ext);
    void setCgiPath(const std::string &path);
    void setReturn(const std::string &url);

    const std::string &getPath() const;
    const std::string &getRoot() const;
    const std::string &getIndex() const;
    bool getAutoindex() const;
    const std::vector<std::string> &getMethods() const;
    const std::string &getCgiExtension() const;
    const std::string &getCgiPath() const;
    const std::string &getReturn() const;

    bool isMethodAllowed(const std::string &method) const;
    void print() const;
    std::string stripSemicolon(const std::string &s);
    std::string intToStr(int n);

};