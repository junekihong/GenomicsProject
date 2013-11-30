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

static inline std::string readString(int sock, const std::string& err)
{
    unsigned length;
    readItem(sock, length, "Error reading the length of " + err);
    char * buff = new char[length] + 1; // TODO leaks on exceptions
    ssize_t bytes_read = recvfrom(sock, buff, length, MSG_WAITALL, NULL, NULL);
    if( bytes_read != length )
        throw std::runtime_error("Error reading " + err);
    std::string result(buff);
    delete []buff;
    return result;
}

static inline void sendString(int sock, const std::string& str, const std::string& err)
{
    unsigned length = static_cast<unsigned>(str.size()); // TODO loses precision
    sendItem(sock, length, "Error sending the length of " + err);
    ssize_t bytes_sent = send(sock, str.data(), str.size(), 0);
    if( bytes_sent != length )
        throw std::runtime_error("Error sending " + err);
}

template<typename T>
static inline void readVector(int sock, const std::string& err, std::vector<T>& result)
{
    unsigned length;
    readItem(sock, length, "Error reading the length of " + err);
    result.resize(length);
    ssize_t bytes_read = recvfrom(sock, result.data(), length *sizeof(T), MSG_WAITALL, NULL, NULL);
    if( bytes_read != length * sizeof(T) )
        throw std::runtime_error("Error reading " + err);
}

template<typename T>
static inline void sendVector(int sock, const std::vector<T>& vec, const std::string& err)
{
    unsigned length = static_cast<unsigned>(vec.size()); // TODO loses precision
    sendItem(sock, length, "Error sending the length of " + err);
    ssize_t bytes_sent = send(sock, vec.data(), vec.size() * sizeof(T), 0);
    if( bytes_sent != length )
        throw std::runtime_error("Error sending " + err);
}

void handle_new_genome(int sock)
{
    std::string name = readString(sock, "the name of a new genome");
    
    unsigned genome_length;
    readItem(sock, genome_length, "Error reading the length of genome " + name);
    
    createGenome(name, genome_length);
}

void handle_new_data(int sock)
{
    std::string name = readString(sock, "the name of the genome currently uploading");
    unsigned startIndex;
    readItem(sock, startIndex, "Error reading the index of the current genome chunk");
    std::vector<char> data;
    readVector(sock, "Error reading genome data", data);
    
    addGenomeData(name, startIndex, data);
}

void handle_genome_info_query(int sock)
{
    std::string name = readString(sock, "the name of the genome in the info request");
    const GenomeInfo& info = getGenomeInfo(name);
    
    message_id_t msg_id = STORE_GENOME_INFO_RESPONSE_ID;
    sendItem(sock, msg_id, "Error sending genome info response ID");
    sendString(sock, info.name, "the name of the genome in the info response");
    sendItem(sock, info.length, "Error sending the length of the genome " + name + " in info response");
}

void handle_genome_content_query(int sock)
{
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
    
}

void handle_query_by_id(int sock)
{
    
}

void handle_query_by_cond(int sock)
{
    
}
