#include <iostream>
#include <set>
#include <stdexcept>

#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

#include "common/listen.h"
#include "common/protocol.h"

#include "cmd_options.h"
#include "genomeOps.h"
#include "solutionOps.h"

// TODO Doesn't handle sockets closing

void handle_new_genome(int sock);
void handle_new_data(int sock);
void handle_new_solution(int sock);
void handle_query_by_id(int sock);
void handle_query_by_cond(int sock);
void handle_genome_info_query(int sock);
void handle_genome_content_query(int sock);

int main(int argc, const char* argv[])
{
    int myport = parse_options(argc, argv);
    
    initializeGenomeSystem();
    initializeSolutionSystem();
    
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
                                    break;
                                case STORE_QUERY_BY_NAME_ID:
                                    handle_genome_content_query(cur_sock);
                                    break;
                                case STORE_GENOME_INFO_QUERY_ID:
                                    handle_genome_info_query(cur_sock);
                                    break;
                                case STORE_GENOME_CONTENT_QUERY_ID:
                                    handle_genome_content_query(cur_sock);
                                    break;
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
#ifdef DEBUG
    std::cout << "mdps.cpp: handling new genome.\n";
#endif

    std::string name = readString(sock, "the name of a new genome");
    
    unsigned genome_length;
    readItem(sock, genome_length, "Error reading the length of genome " + name);
    
    createGenome(name, genome_length);


    message_id_t ack = STORE_QUERY_RESPONSE_ID;
    sendItem(sock, ack, "Error. Could not send back an ACK to the leader for making a new genome.");
}

void handle_new_data(int sock)
{
#ifdef DEBUG
    std::cout << "mdps.cpp: handling new data.\n";
#endif

    std::string name = readString(sock, "the name of the genome currently uploading");
    unsigned startIndex;
    readItem(sock, startIndex, "Error reading the index of the current genome chunk");
    std::vector<char> data;
    readVector(sock, data, "Error reading genome data");
    
    addGenomeData(name, startIndex, data);


    message_id_t ack = STORE_QUERY_RESPONSE_ID;
    sendItem(sock, ack, "Error. Could not send back an ACK to the leader for handling new data.");
}

void handle_genome_info_query(int sock)
{
#ifdef DEBUG
    std::cout << "mdps.cpp: handling genome info query.\n";
#endif

    std::string name = readString(sock, "the name of the genome in the info request");
    const GenomeInfo& info = getGenomeInfo(name);
    
    message_id_t msg_id = STORE_GENOME_INFO_RESPONSE_ID;
    sendItem(sock, msg_id, "Error sending genome info response ID");
    sendString(sock, info.name, "the name of the genome in the info response");
    sendItem(sock, info.length, "Error sending the length of the genome " + name + " in info response");
}

void handle_genome_content_query(int sock)
{
#ifdef DEBUG
    std::cout << "mdps.cpp: handling genome content query.\n";
#endif


    std::string name = readString(sock, "genome name in content request");
    unsigned startIndex, length;
    readItem(sock, startIndex, "Error reading start index in genome content request");
    readItem(sock, length, "Error reading desired length in genome content request");
    
    std::vector<char> data;
    getGenomeData(name, startIndex, length, data);
    
    message_id_t msg_id = STORE_GENOME_CONTENT_RESPONSE_ID;
    sendItem(sock, msg_id, "Error sending genome content response id");
    sendString(sock, name, "the name of the genome in contest response");
    sendItem(sock, startIndex, "Error sending the start index of content response");
    sendVector(sock, data, "the data in the content response");
}

void handle_new_solution(int sock)
{
#ifdef DEBUG
    std::cout << "mdps.cpp: handling new solution.\n";
#endif

    Solution sol;
    ProblemDescription desc;
    readProblemDescription(sock, desc);
    readSolution(sock, sol);
    
    insertSolution(desc, sol);
}

void handle_query_by_id(int sock)
{
#ifdef DEBUG
    std::cout << "mdps.cpp: handling query by id.\n";
#endif

    ProblemID prob;
    bool solution_wanted;
    
    readItem(sock, prob, "Error reading problem ID in query");
    readItem(sock, solution_wanted, "Error reading entire solution flag in query");
    
    QueryResponse resp;
    queryByID(prob, solution_wanted, resp);
    
    message_id_t msg_id = STORE_QUERY_RESPONSE_ID;
    sendItem(sock, msg_id, "Error sending query response id");
    sendQueryResponse(sock, resp);
}

void handle_query_by_cond(int sock)
{
#ifdef DEBUG
    std::cout << "mdps.cpp: handling query by conditions.\n";
#endif

    ProblemDescription prob;
    bool partialsWanted;
    readProblemDescription(sock, prob);
    readItem(sock, partialsWanted, "Error reading whether partial matches should be returned");
    
    QueryResponse resp;
    queryByConditions(prob, partialsWanted, resp);
    
    message_id_t msg_id = STORE_QUERY_RESPONSE_ID;
    sendItem(sock, msg_id, "Error sending query response id");
    sendQueryResponse(sock, resp);
}
