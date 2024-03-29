#include <stdlib.h>
#include <iostream>
#include <map>
#include <set>
#include <sstream>

#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <unistd.h>

#include "common/connect.h"
#include "common/listen.h"
#include "common/protocol.h"
#include "common/util.h"

#include "cmd_options.h"
#include "protocol.h"

StorageProtocol* storage = NULL;
static std::map<int, NetworkHandler*> handlers;
static std::set<int> to_erase;

void destroy_socket(int socket)
{
    to_erase.insert(socket);
}

int main(int argc, const char* argv[])
{
    LeaderConfiguration config = parse_options(argc, argv);
    boost::asio::ip::tcp::iostream storage_stream;
    
#ifndef DEBUG
    try {
#endif
        connect_server(storage_stream, config.storage, "storage");
        storage = new StorageProtocolImpl(storage_stream);
        
        problemNumber = storage->getNextSolutionID();
        storage->getGenomeList(nameToGenomeLength);
        
        int listen_socket = start_listening(config.myport);
        std::cout << "Listening on port " << config.myport << "\n";
        
        fd_set mask, dummy_mask, temp_mask;
        
        FD_ZERO(&mask);
        FD_ZERO(&dummy_mask);
        FD_SET(listen_socket, &mask);
        
        std::set<int> unannounced_sockets;
        
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
                
                to_erase.clear();
                for( std::map<int, NetworkHandler*>::iterator iter = handlers.begin(); iter != handlers.end(); ++iter )
                {
                    NetworkHandler * cur_handler = iter->second;
                    if( FD_ISSET(cur_handler->getSocket(), &temp_mask) )
                    {
                        bool keep = cur_handler->handleNetwork();
                        if( !keep ) {
                            to_erase.insert(iter->first);
                        }
                    }
                }
                for( std::set<int>::iterator iter = to_erase.begin(); iter != to_erase.end(); ++iter )
                {
                    close(*iter);
                    FD_CLR(*iter, &mask);
                    delete handlers[*iter];
                    handlers.erase(*iter);
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
                                    handlers.insert(std::make_pair(cur_sock, new ClientHandler(cur_sock)));
                                    std::cout << "Connected a client.\n";
                                    break;
                                case ANNOUNCE_LEADER:
                                    // Ignore this for now
                                    close(cur_sock);
                                    break;
                                case ANNOUNCE_WORKER:
                                    handlers.insert(std::make_pair(cur_sock, new WorkerHandler(cur_sock)));
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
        delete storage;
#ifndef DEBUG
    }
    catch( const std::exception& err )
    {
        std::cerr << err.what() << "\n";
        delete storage;
        exit(-1);
    }
#endif
    return 0;
}
