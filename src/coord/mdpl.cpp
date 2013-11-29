#include <iostream>
#include <set>
#include <sstream>

#include <sys/socket.h>
#include <netinet/in.h>

#include "cmd_options.h"

int toInt(const std::string& str)
{
    std::stringstream strm(str);
    
    int result;
    strm >> result;
    return result;
}

int start_listening(const std::string& myport_str)
{
    int server_socket;
    int myport = toInt(myport_str);
    
    struct sockaddr_in from_addr;
    from_addr.sin_family = AF_INET;
    from_addr.sin_addr.s_addr = INADDR_ANY;
    from_addr.sin_port = HTONS(myport);
    
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if( server_socket < 0 )
    {
        std::cerr << "Error creating socket\nAborting\n";
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

int main(int argc, const char* argv[])
{
    LeaderConfiguration config = parse_options(argc, argv);
    return 0;
}
