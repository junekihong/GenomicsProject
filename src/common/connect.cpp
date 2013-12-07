#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
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

int start_connection(const char* host, int port)
{
    int s = socket(AF_INET, SOCK_STREAM, 0);
    if( s < 0 ) {
        std::cerr << "Error creating socket\nAborting\n";
        exit(-1);
    }
    
    struct hostent * remote_host = gethostbyname(host);
    int host_num;
    if( remote_host == NULL ) {
        std::cerr << "Error finding hostname\nAborting\n";
        exit(-1);
    }
    memcpy(&host_num, remote_host->h_addr_list[0], sizeof(host_num));
    
    struct sockaddr_in send_addr;
    send_addr.sin_family = AF_INET;
    send_addr.sin_addr.s_addr = host_num;
    send_addr.sin_port = htons(port);
    int ret = connect(s, (struct sockaddr*)&send_addr, sizeof(send_addr));
    if( ret < 0 ) {
        std::cerr << "error connecting\n";
        exit(-1);
    }
    return s;
}
