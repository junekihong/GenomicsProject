#include <iostream>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include "cmd_options.h"

struct Connections
{
    boost::asio::ip::tcp::socket leader;
    boost::asio::ip::tcp::socket storage;

    Connections(boost::asio::io_service& io_service)
        : leader(io_service), storage(io_service)
    { }
};

boost::asio::io_service io_service;

using boost::asio::ip::tcp;

static void connect_server(tcp::socket& socket, const ServerEndpoint& server, tcp::resolver& resolver, const std::string& name)
{
    try {
        tcp::resolver::query query(server.host, server.port);
        tcp::resolver::iterator iter = resolver.resolve(query);

        std::cout << "Attempting to connect to " << name << " at " << server.host << ":" << server.port << "...\n";
        boost::asio::connect(socket, iter);
        std::cout << "Connection succeeded.\n";
    }
    catch( const std::exception& err )
    {
        std::cerr << "Connecting to " << name << " at " << server.host << ":" << server.port << " failed: " << err.what() << "\n";
        std::cerr << "Aborting!\n";
        exit(-1);
    }
}

void connect_to_servers(const WorkerConfiguration& config, Connections& conns)
{
    tcp::resolver resolver(io_service);
    connect_server(conns.leader, config.leader, resolver, "leader");
    connect_server(conns.storage, config.storage, resolver, "storage");
}

int main(int argc, const char* argv[])
{
    WorkerConfiguration config = parse_options(argc, argv);

    Connections conns(io_service);

    try {
        connect_to_servers(config, conns);
    }
    catch( const std::exception& err)
    {
        std::cerr << err.what() << "\n";
        return -1;
    }
    
    return 0;
}
