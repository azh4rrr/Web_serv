#include "config/ParssingConf.hpp"
#include <iostream>

int main(int argc, char *argv[]) {

    (void)argc;
    ParssingConf ParssingConf;
    try
    {
        ParssingConf.parseConfig(argv[1]);
        // ParssingConf.validate();
        // ParssingConf.print();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;

}