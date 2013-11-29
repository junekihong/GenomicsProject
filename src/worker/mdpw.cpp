#include <iostream>

#include "common/connect.h"
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
