#include <iostream>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include "cmd_options.h"

#include "protocol.h"
#include "protocol_impl.h"
#include "worker.h"

struct Connections
{
    boost::asio::ip::tcp::iostream leader;
    boost::asio::ip::tcp::iostream storage;

    Connections(boost::asio::io_service&)
        : leader(), storage()
    { }
};

boost::asio::io_service io_service;

using boost::asio::ip::tcp;

static void connect_server(tcp::iostream& socket, const ServerEndpoint& server, tcp::resolver& resolver, const std::string& name)
{
    try {
        tcp::resolver::query query(server.host, server.port);
        tcp::resolver::iterator iter = resolver.resolve(query);

        std::cout << "Attempting to connect to " << name << " at " << server.host << ":" << server.port << "...\n";
        socket.connect(server.host, server.port);
        if( !socket ) {
            std::cerr << "Connecting to " << name << " at " << server.host << ":" << server.port << " failed: " << socket.error().message() << "\n";
            std::cerr << "Aborting!\n";
            exit(-1);
        }
        else {
            std::cout << "Connection succeeded.\n";
        }
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

    WorkerProtocolImpl leader(conns.leader);
    StorageProtocolImpl storage(conns.storage);
    runWorker(leader, storage);
    
    return 0;
}
