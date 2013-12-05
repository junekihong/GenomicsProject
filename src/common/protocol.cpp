#include <vector>

#include <sys/socket.h>

#include <msgpack.hpp>

#include "common/util.h"

#include "protocol.h"
#include "problem.h"

void receive_ack(std::istream&socket, message_id_t expected_code)
{
    msgpack::unpacker unpack;
    readBuffer(socket, unpack);
    message_id_t code;
    read(unpack, code);
    if( code != expected_code )
        throw std::runtime_error(std::string("Expected ack, but got another one: ") + toString(code));
}
void send_ack(int sock, message_id_t ack_code)
{
    msgpack::sbuffer sbuf;
    message_id_t ack = ack_code;
    msgpack::pack(&sbuf, ack);
    sendBuffer(sock, sbuf);
}
void send_ack(std::ostream& sock, message_id_t ack_code)
{
    msgpack::sbuffer sbuf;
    message_id_t ack = ack_code;
    msgpack::pack(&sbuf, ack);
    sendBuffer(sock, sbuf);
}


//name is the name of the string
//length is the length of the genome
void StorageProtocolImpl::createNewGenome(const std::string& name, unsigned length)
{
    msgpack::sbuffer buff;
    msgpack::pack(&buff, static_cast<message_id_t>(STORE_NEW_GENOME_ID));
    msgpack::pack(&buff, name);
    msgpack::pack(&buff, length);
    sendBuffer(socket, buff);
    
    receive_ack(socket, STORE_QUERY_RESPONSE_ID);
}

void StorageProtocolImpl::insertGenomeData(const std::string& name, unsigned& index, const std::vector<unsigned char>& data)
{
    msgpack::sbuffer sbuf;
    msgpack::pack(&sbuf, static_cast<message_id_t>(STORE_NEW_DATA_ID));
    msgpack::pack(&sbuf, name); // TODO. Whoever is listening on the other end needs to read the name in as well.
    msgpack::pack(&sbuf, index);
    msgpack::pack(&sbuf, data);
    sendBuffer(socket, sbuf);
    
    receive_ack(socket, STORE_QUERY_RESPONSE_ID);
}

bool StorageProtocolImpl::insertSolution(const ProblemDescription& prob, const Solution& solution)
{
    msgpack::sbuffer sbuf;
    msgpack::pack(&sbuf, static_cast<message_id_t>(STORE_NEW_SOLUTION_ID));
    msgpack::pack(&sbuf, prob);
    msgpack::pack(&sbuf, solution);
    sendBuffer(socket, sbuf);
    
    receive_ack(socket, STORE_QUERY_RESPONSE_ID);
    return true;
}

QueryResponse* StorageProtocolImpl::queryByProblemID(const ProblemID& problemID, bool entireSolution)
{
    msgpack::sbuffer sbuf;
    msgpack::pack(&sbuf, static_cast<message_id_t>(STORE_QUERY_BY_ID_ID));
    msgpack::pack(&sbuf, problemID);
    msgpack::pack(&sbuf, entireSolution);
    sendBuffer(socket, sbuf);
    
    msgpack::unpacker unpack;
    readBuffer(socket, unpack);
    
    message_id_t responseMessage;
    read(unpack, responseMessage);
    if(responseMessage != STORE_QUERY_RESPONSE_ID)
    {
        std::cerr << "Error. Storage did not properly respond to our query by problemID\n";
        return NULL;
    }
    
    QueryResponse* response = new QueryResponse();
    read(unpack, *response);
    
	return response;
}

QueryResponse* StorageProtocolImpl::queryByInitialConditions(const ProblemDescription& problemDescription, const bool wantPartials)
{
    msgpack::sbuffer sbuf;
    msgpack::pack(&sbuf, static_cast<message_id_t>(STORE_QUERY_BY_COND_ID));
    msgpack::pack(&sbuf, problemDescription);
    msgpack::pack(&sbuf, wantPartials);
    sendBuffer(socket, sbuf);
    
    msgpack::unpacker unpack;
    readBuffer(socket, unpack);
    
    message_id_t responseMessage;
    read(unpack, responseMessage);
    if(responseMessage != STORE_QUERY_RESPONSE_ID)
        throw std::runtime_error("Error. Storage did not properly respond to our query by initial conditians");
    
    QueryResponse* response = new QueryResponse();
    read(unpack, *response);
    return response;
}

std::vector<unsigned char> StorageProtocolImpl::queryByName(const std::string& name, int startIndex, int length)
{
    msgpack::sbuffer sbuf;
    msgpack::pack(&sbuf, static_cast<message_id_t>(STORE_GENOME_CONTENT_QUERY_ID));
    msgpack::pack(&sbuf, name);
    msgpack::pack(&sbuf, startIndex);
    msgpack::pack(&sbuf, length);
    sendBuffer(socket, sbuf);
    
    msgpack::unpacker unpack;
    readBuffer(socket, unpack);
    
    message_id_t responseMessage;
    read(unpack, responseMessage);
    if(responseMessage != STORE_GENOME_CONTENT_RESPONSE_ID)
        throw std::runtime_error("Error. Storage did not properly respond to our query by name");

    std::string responseName;
    int responseStartIndex;
    std::vector<unsigned char> genome;
    
    read(unpack, responseName);
    read(unpack, responseStartIndex);
    read(unpack, genome);

    return genome;
}

ProblemID StorageProtocolImpl::getNextSolutionID()
{
    msgpack::sbuffer sbuf;
    msgpack::pack(&sbuf, static_cast<message_id_t>(STORE_MAX_SOL_REQUEST_ID));
    sendBuffer(socket, sbuf);
    
    msgpack::unpacker unpack;
    readBuffer(socket, unpack);
#ifdef DEBUG
    printBuffer(std::cerr, unpack.buffer(), unpack.used);
#endif
    
    message_id_t msg_id;
    read(unpack, msg_id);
    if( msg_id != STORE_MAX_SOL_RESPONSE_ID ) {
        printBuffer(std::cerr, unpack.buffer(), unpack.used);
        throw std::runtime_error(std::string("Storage responded to max solution ID request with message type ") + toString(msg_id));
    }

    
    ProblemID id;
    read(unpack, id);
    return id;
}

void StorageProtocolImpl::getGenomeList(std::map<std::string, int>& genome_list)
{
    msgpack::sbuffer sbuf;
    msgpack::pack(&sbuf, static_cast<message_id_t>(GENOME_LIST_REQUEST_ID));
    sendBuffer(socket, sbuf);
    
    msgpack::unpacker unpack;
    readBuffer(socket, unpack);
    
    message_id_t responseMessage;
    read(unpack, responseMessage);
    if( responseMessage != GENOME_LIST_RESPONSE_ID )
        throw std::runtime_error(std::string("Storage responded to genome list request with message type ") + toString(responseMessage));
    
    std::map<std::string, GenomeInfo> tmp_list;
    read(unpack, tmp_list);
    for( std::map<std::string, GenomeInfo>::iterator iter = tmp_list.begin(); iter != tmp_list.end(); ++iter)
        genome_list.insert(std::make_pair(iter->first, iter->second.length));
}

void readBuffer(std::istream& socket, msgpack::unpacker& unpack)
{
    unsigned msg_size;
    socket.read(reinterpret_cast<char*>(&msg_size), sizeof(msg_size));
    unpack.reserve_buffer(msg_size);
    socket.read(unpack.buffer(), msg_size);
    unpack.buffer_consumed(msg_size);
}
void readBuffer(int socket, msgpack::unpacker& unpack)
{
    unsigned msg_size;
    ssize_t bytes_read = recvfrom(socket, &msg_size, sizeof(msg_size), MSG_WAITALL, NULL, NULL);
    if( bytes_read != sizeof(msg_size) )
        throw std::runtime_error("Error reading size of message");
#ifdef DEBUG
    std::cout << "Reading buffer of size " << msg_size << " bytes\n";
#endif
    unpack.reserve_buffer(msg_size);
    bytes_read = recvfrom(socket, unpack.buffer(), msg_size, MSG_WAITALL, NULL, NULL);
    if( bytes_read != msg_size )
        throw std::runtime_error("Error reading message");
    unpack.buffer_consumed(msg_size);
}

void sendBuffer(std::ostream& socket, const msgpack::sbuffer& buff)
{
#ifdef DEBUG
    std::cerr << "Sending buffer length " << buff.size() << ": ";
    printBuffer(std::cerr, buff.data(), buff.size());
#endif
    unsigned length = static_cast<unsigned>(buff.size());
    socket.write(reinterpret_cast<const char*>(&length), sizeof(length));
    socket.write(buff.data(), buff.size());
}

void sendBuffer(int socket, const msgpack::sbuffer& buff)
{
    unsigned length = static_cast<unsigned>(buff.size());
    send(socket, &length, sizeof(length), 0);
    send(socket, buff.data(), buff.size(), 0);
}

#include <iomanip>

void printBuffer(std::ostream& out, const char * data, unsigned long length)
{
    out << std::hex;
    if( length )
        out << static_cast<int>(data[0]);
    for( unsigned i = 1; i < length; ++i )
        out << " " << static_cast<int>(data[i]);
    out << "\n";
    out << std::dec;
}
