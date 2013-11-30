#include <iostream>

#include <netinet/in.h>
#include <sys/socket.h>

#include "listen.h"
#include "util.h"

int start_listening(const std::string& myport_str)
{
    int server_socket;
    long on = 1;
    int myport = toInt(myport_str);
    
    struct sockaddr_in from_addr;
    from_addr.sin_family = AF_INET;
    from_addr.sin_addr.s_addr = INADDR_ANY;
    from_addr.sin_port = htons(myport);
    
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if( server_socket < 0 )
    {
        std::cerr << "Error creating socket\nAborting\n";
        exit(-1);
    }
    
    if( setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on)) < 0 )
    {
        std::cerr << "Error setting socket to SO_REUSEADDR\n";
        exit(-1);
    }
    
    if ( bind( server_socket, (struct sockaddr *)&from_addr, sizeof(from_addr) ) < 0 )
    {
        std::cerr << "Error binding to port " << myport << "\n";
        std::cerr << "Aborting\n";
        exit(-1);
    }
    
    if( listen(server_socket, 4) < 0 )
    {
        std::cerr << "Error listening\nAborting\n";
        exit(-1);
    }
    
    return server_socket;
}
