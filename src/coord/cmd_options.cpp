#include <iostream>
#include <string>
#include <boost/program_options.hpp>

namespace po = boost::program_options;

#include "cmd_options.h"

LeaderConfiguration parse_options(int argc, const char* argv[])
{
    LeaderConfiguration config;
    std::string storage_address;
    po::options_description desc("MDP Worker Options");
    desc.add_options()
    ("help", "print this help message")
    ("[prt", po::value<std::string>(&config.myport)->default_value(DEFAULT_PORT),
     "the port number of this coordinator should listen one")
    ("storage", po::value<std::string>(&storage_address)->default_value(DEFAULT_SERVER ":" DEFAULT_STORAGE_PORT),
     "the hostname and, optionally, the port number of a storage server")
    ;
    
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);
    
    if( vm.count("help") )
    {
        std::cout << desc << "\n";
        exit(0);
    }
    
    try {
        extract_server_port(storage_address, DEFAULT_STORAGE_PORT, config.storage);
    }
    catch( std::runtime_error err )
    {
        std::cerr << err.what() << "\n";
        exit(-1);
    }
    
    return config;
}
