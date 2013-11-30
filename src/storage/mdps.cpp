#include <iostream>
#include <set>
#include <stdexcept>

#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

#include "common/listen.h"
#include "common/protocol.h"

#include "cmd_options.h"

// TODO Doesn't handle sockets closing

void handle_new_genome(int sock);
void handle_new_data(int sock);
void handle_new_solution(int sock);
void handle_query_by_id(int sock);
void handle_query_by_cond(int sock);

int main(int argc, const char* argv[])
{
    int myport = parse_options(argc, argv);
    
    try {
        int listen_socket = start_listening(myport);
        
        fd_set mask, dummy_mask, temp_mask;
        
        FD_ZERO(&mask);
        FD_ZERO(&dummy_mask);
        FD_SET(listen_socket, &mask);
        
        std::set<int> sockets;
        
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
                    sockets.insert(new_socket);
                }
                
                for( std::set<int>::iterator iter = sockets.begin(); iter != sockets.end(); ++iter )
                {
                    int cur_sock = *iter;
                    if( FD_ISSET(cur_sock, &temp_mask) )
                    {
                        message_id_t msg_id;
                        ssize_t bytes_read = recvfrom(cur_sock, &msg_id, sizeof(msg_id), MSG_WAITALL, NULL, NULL);
                        if( bytes_read != sizeof(msg_id) )
                        {
                            std::cerr << "Error reading a message id\n";
                            close(cur_sock);
                            FD_CLR(cur_sock, &mask);
                        }
                        else {
                            switch( msg_id )
                            {
                                case STORE_NEW_GENOME_ID:
                                    handle_new_genome(cur_sock);
                                    break;
                                case STORE_NEW_DATA_ID:
                                    handle_new_data(cur_sock);
                                    break;
                                case STORE_NEW_SOLUTION_ID:
                                    handle_new_solution(cur_sock);
                                    break;
                                case STORE_QUERY_BY_ID_ID:
                                    handle_query_by_id(cur_sock);
                                    break;
                                case STORE_QUERY_BY_COND_ID:
                                    handle_query_by_cond(cur_sock);
                                default:
                                    std::cerr << "Unknown message type: " << msg_id << "\n";
                            }
                        }
                    }
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

void handle_new_genome(int sock)
{
    
}

void handle_new_data(int sock)
{
    
}

void handle_new_solution(int sock)
{
    
}

void handle_query_by_id(int sock)
{
    
}

void handle_query_by_cond(int sock)
{
    
}
