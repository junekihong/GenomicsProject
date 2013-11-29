#include <boost/asio/connect.hpp>

#include "connect.h"

using boost::asio::ip::tcp;

void connect_server(tcp::iostream& socket, const ServerEndpoint& server, const std::string& name)
{
    try {
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

