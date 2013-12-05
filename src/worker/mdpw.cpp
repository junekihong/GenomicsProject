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

    Connections()
        : leader(), storage()
    { }
};

using boost::asio::ip::tcp;

void connect_to_servers(const WorkerConfiguration& config, Connections& conns)
{
    connect_server(conns.leader, config.leader, "leader");
    int announce = ANNOUNCE_WORKER;
    conns.leader.write(reinterpret_cast<const char*>(&announce), sizeof(announce));
    if( !conns.leader ) {
        throw std::runtime_error("Error sending worker announcement");
    }
    connect_server(conns.storage, config.storage, "storage");
}

int main(int argc, const char* argv[])
{
    WorkerConfiguration config = parse_options(argc, argv);

    Connections conns;

#ifndef DEBUG
    try {
#endif
        connect_to_servers(config, conns);
        
        WorkerProtocolImpl leader(conns.leader);
        StorageProtocolImpl storage(conns.storage);
        runWorker(leader, storage);
#ifndef DEBUG
    }
    catch( const std::exception& err)
    {
        std::cerr << err.what() << "\n";
        return -1;
    }
#endif

    return 0;
}
