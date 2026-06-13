#include "config/ParssingConf.hpp"
#include "Webserv.hpp"
#include <iostream>

int main(int argc, char *argv[]) {

    (void)argc;
    // ParssingConf ParssingConf;
    Webserv  Webserv;
    try
    {
        std::cout << "Setting up servers..." << std::endl;
        std::cout << "Config file: " << argv[1] << std::endl;
        Webserv.setupServers(argv[1]);
        
        Webserv.Start();
        // ParssingConf.parseConfig(argv[1]);
        
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;

}