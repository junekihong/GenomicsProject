#include <iostream>
#include <string>
#include <boost/program_options.hpp>

namespace po = boost::program_options;

#include "common/cmd_options.h"

#include "cmd_options.h"

int parse_options(int argc, const char* argv[])
{
    int myport;
    po::options_description desc("MDP Storage Options");
    desc.add_options()
    ("help", "print this help message")
    ("port", po::value<int>(&myport)->default_value(DEFAULT_STORAGE_PORT_INT),
     "the port number of this storage node should listen one")
    ;
    
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);
    
    if( vm.count("help") )
    {
        std::cout << desc << "\n";
        exit(0);
    }
    
    return myport;
}
