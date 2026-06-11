#pragma once

#include <vector>
#include <string>
#include "Config.hpp"

class ParssingConf
{
private:
    std::vector<Config> _configs;

    
    std::string readFile(const std::string &filename);
    std::string removeComments(const std::string &content);
    std::vector<std::string> tokenize(const std::string &content);
    Config parseServerBlock(const std::vector<std::string> &tokens, size_t &i);
    LocationConfig parseLocationBlock(const std::vector<std::string> &tokens, size_t &i, const std::string &path);
    void validate();
    void error(const std::string &msg);

public:
    const std::vector<Config> &getConfigs() const;
    void parseConfig(const std::string &filename);
    void print() const;
};
