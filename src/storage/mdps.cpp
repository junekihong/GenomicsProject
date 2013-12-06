#include <iostream>
#include <set>
#include <stdexcept>

#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

#include <msgpack.hpp>

#include "common/listen.h"
#include "common/protocol.h"

#include "cmd_options.h"
#include "genomeOps.h"
#include "solutionOps.h"

// TODO Doesn't handle sockets closing

void handle_new_genome(int sock, msgpack::unpacker& unpack);
void handle_new_data(int sock, msgpack::unpacker& unpack);
void handle_new_solution(int sock, msgpack::unpacker& unpack);
void handle_query_by_id(int sock, msgpack::unpacker& unpack);
void handle_query_by_cond(int sock, msgpack::unpacker& unpack);
void handle_genome_info_query(int sock, msgpack::unpacker& unpack);
void handle_genome_content_query(int sock, msgpack::unpacker& unpack);
void handle_max_solution(int sock, msgpack::unpacker& unpack);
void handle_genome_list(int sock, msgpack::unpacker& unpack);

int main(int argc, const char* argv[])
{
    int myport = parse_options(argc, argv);
    
    std::cout << "Starting genomes\n";
    std::cout.flush();
    initializeGenomeSystem();
    std::cout << "Starting solution\n";
    initializeSolutionSystem();
    std::cout << "Got solutions\n";
    
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
            std::cout << "going in to select\n";
            int num = select(FD_SETSIZE, &temp_mask, &dummy_mask, &dummy_mask, NULL);
            std::cout << "awoke from select!\n";
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
                        msgpack::unpacker unpack;
                        readBuffer(cur_sock, unpack);
                    
                        message_id_t msg_id;
                        read(unpack, msg_id);
                        switch( msg_id )
                        {
                            case STORE_NEW_GENOME_ID:
                                handle_new_genome(cur_sock, unpack);
                                break;
                            case STORE_NEW_DATA_ID:
                                handle_new_data(cur_sock, unpack);
                                break;
                            case STORE_NEW_SOLUTION_ID:
                                handle_new_solution(cur_sock, unpack);
                                break;
                            case STORE_QUERY_BY_ID_ID:
                                handle_query_by_id(cur_sock, unpack);
                                break;
                            case STORE_QUERY_BY_COND_ID:
                                handle_query_by_cond(cur_sock, unpack);
                                break;
                            case STORE_GENOME_INFO_QUERY_ID:
                                handle_genome_info_query(cur_sock, unpack);
                                break;
                            case STORE_GENOME_CONTENT_QUERY_ID:
                                handle_genome_content_query(cur_sock, unpack);
                                break;
                            case STORE_MAX_SOL_REQUEST_ID:
                                handle_max_solution(cur_sock, unpack);
                                break;
                            case GENOME_LIST_REQUEST_ID:
                                handle_genome_list(cur_sock, unpack);
                                break;
                            default:
                                std::cerr << "Unknown message type: " << msg_id << " from socket " << cur_sock << "\n";
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

void handle_new_genome(int sock, msgpack::unpacker& unpack)
{
#ifdef DEBUG
    std::cout << "mdps.cpp: handling new genome.\n";
#endif
    std::string name;
    unsigned genome_length;
    
    read(unpack, name);
    read(unpack, genome_length);
    
    createGenome(name, genome_length);

    send_ack(sock, STORE_QUERY_RESPONSE_ID);
}

void handle_new_data(int sock, msgpack::unpacker& unpack)
{
#ifdef DEBUG
    std::cout << "mdps.cpp: handling new data.\n";
#endif
    std::string name;
    unsigned startIndex;
    std::vector<unsigned char> data;
    
    read(unpack, name);
    read(unpack, startIndex);
    read(unpack, data);
    
    addGenomeData(name, startIndex, data);

    send_ack(sock, STORE_QUERY_RESPONSE_ID);
}

void handle_genome_info_query(int sock, msgpack::unpacker& unpack)
{
#ifdef DEBUG
    std::cout << "mdps.cpp: handling genome info query.\n";
#endif
    std::string name;
    read(unpack, name);
    
    const GenomeInfo& info = getGenomeInfo(name);
    
    
    msgpack::sbuffer sbuf;
    message_id_t msg_id = STORE_GENOME_INFO_RESPONSE_ID;
    msgpack::pack(&sbuf, msg_id);
    msgpack::pack(&sbuf, info);
    sendBuffer(sock, sbuf);
}

void handle_genome_content_query(int sock, msgpack::unpacker& unpack)
{
#ifdef DEBUG
    std::cout << "mdps.cpp: handling genome content query from socket " << sock << ".\n";
#endif
    std::string name;
    unsigned startIndex, length;
    
    read(unpack, name);
    read(unpack, startIndex);
    read(unpack, length);
    
    std::vector<unsigned char> data;
    getGenomeData(name, startIndex, length, data);
    
    msgpack::sbuffer sbuf;
    message_id_t msg_id = STORE_GENOME_CONTENT_RESPONSE_ID;
    msgpack::pack(&sbuf, msg_id);
    msgpack::pack(&sbuf, name);
    msgpack::pack(&sbuf, startIndex);
    msgpack::pack(&sbuf, data);
    sendBuffer(sock, sbuf);
}

void handle_new_solution(int sock, msgpack::unpacker& unpack)
{
#ifdef DEBUG
    std::cout << "mdps.cpp: handling new solution from socket " << sock << ".\n";
#endif
    ProblemDescription desc;
    Solution sol;
    
    read(unpack, desc);
    read(unpack, sol);
    std::cout << "Answer dimentsions: " << sol.matrix.getLength() << " x " << sol.matrix.getWidth() << "\n";
    
    insertSolution(desc, sol);
    
    // Send an ACK
    // FIXME Paul doesn't like this ACK code, or the ACK in general, but that's another story
    send_ack(sock, STORE_QUERY_RESPONSE_ID);
}

void handle_query_by_id(int sock, msgpack::unpacker& unpack)
{
#ifdef DEBUG
    std::cout << "mdps.cpp: handling query by id.\n";
#endif
    ProblemID prob;
    bool solution_wanted;
    
    read(unpack, prob);
    read(unpack, solution_wanted);
    
    QueryResponse resp;
    queryByID(prob, solution_wanted, resp);
    
    msgpack::sbuffer sbuf;
    message_id_t msg_id = STORE_QUERY_RESPONSE_ID;
    msgpack::pack(&sbuf, msg_id);
    msgpack::pack(&sbuf, resp);
    sendBuffer(sock, sbuf);
}

void handle_query_by_cond(int sock, msgpack::unpacker& unpack)
{
#ifdef DEBUG
    std::cout << "mdps.cpp: handling query by conditions.\n";
#endif
    ProblemDescription prob;
    bool partialsWanted;
    
    read(unpack, prob);
    read(unpack, partialsWanted);
    
    QueryResponse resp;
    queryByConditions(prob, partialsWanted, resp);
    
    msgpack::sbuffer sbuf;
    message_id_t msg_id = STORE_QUERY_RESPONSE_ID;
    msgpack::pack(&sbuf, msg_id);
    msgpack::pack(&sbuf, resp);
    sendBuffer(sock, sbuf);
}

void handle_max_solution(int sock, msgpack::unpacker& unpack)
{
#ifdef DEBUG
    std::cout << "mdps.cpp: handling next solution id request\n";
#endif
    
    msgpack::sbuffer sbuf;
    message_id_t msg_id = STORE_MAX_SOL_RESPONSE_ID;
    msgpack::pack(&sbuf, msg_id);
    msgpack::pack(&sbuf, nextSolutionID);
#ifdef DEBUG
    printBuffer(std::cout, sbuf.data(), sbuf.size());
#endif
    sendBuffer(sock, sbuf);
}

void handle_genome_list(int sock, msgpack::unpacker& unpack)
{
#ifdef DEBUG
    std::cout << "mdps.cpp: handling genone list request\n";
#endif
    msgpack::sbuffer sbuf;
    message_id_t msg_id = GENOME_LIST_RESPONSE_ID;
    msgpack::pack(&sbuf, msg_id);
    msgpack::pack(&sbuf, genomes);
    sendBuffer(sock, sbuf);
}
