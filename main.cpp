// #include "config/ParssingConf.hpp"
#include "Webserv.hpp"
#include <iostream>

int main(int argc, char **argv) {

    if(argc == 1 || argc == 2)
    {
        std::string configFile ;
        if (argc == 1) {
            configFile = "config/default.conf";
        } else {
            configFile = argv[1];
        }
        Webserv  Webserv;
        try
        {            
            std::cout << "Setting up servers..." << std::endl;
            // std::cout << "Config file: " << configFile << std::endl;
            Webserv.setupServers(configFile);
            // Webserv.PrintServers();
            Webserv.Start();
            // ParssingConf.parseConfig(configFile); 
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << std::endl;
            return 1;
        }   
    }
    else
    {
        std::cerr << "Usage: " << argv[0] << " [config_file]" << std::endl;
        return 1;
    }
}