#include "config/ParssingConf.hpp"
#include "Webserv.hpp"
#include <iostream>

int main(int argc, char *argv[]) {

    (void)argc;
    // ParssingConf ParssingConf;
    Webserv  Webserv;
    try
    {
        Webserv.setupServers(argv[1]);
        // ParssingConf.parseConfig(argv[1]);
        
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;

}