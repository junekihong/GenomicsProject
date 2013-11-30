#include <stdlib.h>
#include <iostream>
#include <set>
#include <sstream>

#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <unistd.h>

#include "cmd_options.h"
#include "common/protocol.h"
#include "common/util.h"

#include "protocol.h"

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

int main(int argc, const char* argv[])
{
    LeaderConfiguration config = parse_options(argc, argv);
    
    try {
        int listen_socket = start_listening(config.myport);
        std::cout << "Listening on port " << config.myport << "\n";
        
        fd_set mask, dummy_mask, temp_mask;
        
        FD_ZERO(&mask);
        FD_ZERO(&dummy_mask);
        FD_SET(listen_socket, &mask);
        
        std::set<int> unannounced_sockets;
        std::set<NetworkHandler*> handlers;
        
        for(;;)
        {
            temp_mask = mask;
            int num = select(FD_SETSIZE, &temp_mask, &dummy_mask, &dummy_mask, NULL);
            if( num > 0 )
            {
                if( FD_ISSET(listen_socket, &temp_mask) )
                {
                    int new_socket = accept(listen_socket, 0, 0);
                    FD_SET(new_socket, &mask);
                    unannounced_sockets.insert(new_socket);
                }
                
                std::set<NetworkHandler*> to_erase;
                for( std::set<NetworkHandler*>::iterator iter = handlers.begin(); iter != handlers.end(); ++iter )
                {
                    NetworkHandler * cur_handler = *iter;
                    if( FD_ISSET(cur_handler->getSocket(), &temp_mask) )
                    {
                        bool keep = cur_handler->handleNetwork();
                        if( !keep ) {
                            to_erase.insert(cur_handler);
                        }
                    }
                }
                for( std::set<NetworkHandler*>::iterator iter = to_erase.begin(); iter != to_erase.end(); ++iter )
                {
                    delete *iter;
                    handlers.erase(*iter);
                    close((*iter)->getSocket());
                    FD_CLR((*iter)->getSocket(), &mask);
                }
                
                std::set<int> sockets_to_erase;
                for( std::set<int>::iterator iter = unannounced_sockets.begin(); iter != unannounced_sockets.end(); ++iter )
                {
                    int cur_sock = *iter;
                    if( FD_ISSET(cur_sock, &temp_mask) )
                    {
                        int announce_type = 0;
                        ssize_t bytes_read = recvfrom(cur_sock, &announce_type, sizeof(announce_type), MSG_WAITALL, NULL, NULL);
                        if( bytes_read != sizeof(announce_type) )
                        {
                            std::cerr << "Error reading an announcement\n";
                            close(cur_sock);
                            FD_CLR(cur_sock, &mask);
                        }
                        else
                        {
                            switch(announce_type)
                            {
                                case ANNOUNCE_CLIENT:
                                    handlers.insert(new ClientHandler(cur_sock));
                                    std::cout << "Connected a client.\n";
                                    break;
                                case ANNOUNCE_LEADER:
                                    // Ignore this for now
                                    close(cur_sock);
                                    break;
                                case ANNOUNCE_WORKER:
                                    handlers.insert(new WorkerHandler(cur_sock));
                                    std::cout << "Connected a worker.\n";
                                    break;
                                case ANNOUNCE_STORAGE:
                                    // This should probably be a no-op
                                    break;
                                default:
                                    std::cerr << "Unknown announcement: " << announce_type << "\t" <<
                                        (static_cast<char>(announce_type & 0xff)) << " " <<
                                        (static_cast<char>((announce_type >> 8) & 0xff)) << " " <<
                                        (static_cast<char>((announce_type >> 16) & 0xff)) << " " <<
                                        (static_cast<char>((announce_type >> 24) & 0xff)) << " "
                                        <<"\n";
                                    break;
                            }
                        }
                        sockets_to_erase.insert(cur_sock);
                    }
                }
                for( std::set<int>::iterator iter = sockets_to_erase.begin(); iter != sockets_to_erase.end(); ++iter )
                {
                    unannounced_sockets.erase(*iter);
                }

            }
        }
    }
    catch( const std::exception& err )
    {
        std::cerr << err.what() << "\n";
        exit(-1);
    }
    return 0;
}
