#ifndef __PROTOCOL_COMMON_H__
#define __PROTOCOL_COMMON_H__

#include <iostream>
#include <stdexcept>

#include <boost/asio/ip/tcp.hpp>

#include <msgpack.hpp>

#include "location.h"
#include "problem.h"
#include "solution.h"

#define ANNOUNCE_CLIENT                      1
#define ANNOUNCE_LEADER                      2
#define ANNOUNCE_WORKER                      3
#define ANNOUNCE_STORAGE                     4

#define PROBLEM_LIST_REQUEST_ID              5
#define PROBLEM_LIST_RESPONSE_ID             5

#define PROBLEM_CLAIM_REQUEST_ID             6
#define PROBLEM_CLAIM_RESPONSE_ID            6

#define SOLUTION_REPORT_ID                   7

#define GENOME_UPLOAD_START_ID               8
#define UPLOAD_REQUEST_RECIEVED_ID           8

#define GENOME_LIST_REQUEST_ID               9
#define GENOME_LIST_RESPONSE_ID              9

#define LOCAL_ALIGN_START_ID                10
#define LOCAL_ALIGN_FINISH_ID               11

#define STORE_NEW_GENOME_ID                 12
#define STORE_NEW_DATA_ID                   13
#define STORE_NEW_SOLUTION_ID               14
#define STORE_QUERY_BY_ID_ID                15
#define STORE_QUERY_BY_COND_ID              16

#define STORE_GENOME_INFO_QUERY_ID          18
#define STORE_GENOME_INFO_RESPONSE_ID       18

#define STORE_GENOME_CONTENT_QUERY_ID       19
#define STORE_GENOME_CONTENT_RESPONSE_ID    19

#define STORE_QUERY_RESPONSE_ID             20

#define STORE_MAX_SOL_REQUEST_ID            21
#define STORE_MAX_SOL_RESPONSE_ID           21

#define BUFF_SIZE 32*1024

class ProblemDescription;

typedef int message_id_t;

void readBuffer(int socket, msgpack::unpacker& buff);
void readBuffer(std::istream& socket, msgpack::unpacker& unpack);
void sendBuffer(int socket, const msgpack::sbuffer& buff);
void sendBuffer(std::ostream& socket, const msgpack::sbuffer& buff);

void receive_ack(std::istream&socket, message_id_t expected_code);
void send_ack(int sock, message_id_t ack_code);
void send_ack(std::ostream& sock, message_id_t ack_code);

void printBuffer(std::ostream& out, const char * data, unsigned long length);

template<typename T>
void read(msgpack::unpacker& unpack, T& out_param, const std::string& err = "Error unpacking data")
{
    msgpack::unpacked result;
    if( !unpack.next(&result) ) {
        printBuffer(std::cerr, unpack.buffer(), unpack.used);
        throw std::runtime_error(err);
    }
    out_param = result.get().as<T>();
}

class GenomeInfo
{
public:
    std::string name;
    unsigned length;
    
    GenomeInfo() // needed for msgpack
    : name(), length()
    { }
    
    GenomeInfo(std::string n, unsigned l)
    : name(n), length(l)
    { }
    
    GenomeInfo(const GenomeInfo& other)
    : name(other.name), length(other.length)
    { }
    
    MSGPACK_DEFINE(name, length);
};

class QueryResponse
{
public:
    // True if a match was found
    bool success;
    // True if an exact match was found
    bool exactMatch;
    
    // Given problem description.
    ProblemDescription problemDescription;
    
    // Solution description. Maximum value and location in the matrix.
    int maxValue;
    Location location;
    
    // The solution.
    // This may be omitted
    Solution * sol;
    
    QueryResponse()
    : success(false), exactMatch(false), problemDescription(), maxValue(-1),
      location(), sol(NULL)
    { }
    
    ~QueryResponse()
    {
        if( sol )
            delete sol;
    }
    
    // TODO figure out a way to send back solution certificates
    
    //MSGPACK_DEFINE(success, exactMatch, problemDescription, maxValue, location);
    // TODO wrap solution
    template <typename Packer>
	void msgpack_pack(Packer& pk) const
	{
        if( sol == NULL )
            pk.pack_array(6);
        else
            pk.pack_array(7);
        pk.pack(success);
        pk.pack(exactMatch);
        pk.pack(problemDescription);
        pk.pack(maxValue);
        pk.pack(location);
        pk.pack(sol != NULL);
        if( sol != NULL )
            pk.pack(*sol);
	}
	void msgpack_unpack(msgpack::object o)
	{
        if( o.type != msgpack::type::ARRAY )
            throw msgpack::type_error();
        bool hasSolution;
        const size_t size = o.via.array.size;
        if( size < 6 ) throw msgpack::type_error();
        
        o.via.array.ptr[0].convert(&success);
        o.via.array.ptr[1].convert(&exactMatch);
        o.via.array.ptr[2].convert(&problemDescription);
        o.via.array.ptr[3].convert(&maxValue);
        o.via.array.ptr[4].convert(&location);
        o.via.array.ptr[5].convert(&hasSolution);
        if( 6 + (hasSolution ? 1 : 0) != size )
            throw msgpack::type_error();
        if( hasSolution ) {
            sol = new Solution;
            o.via.array.ptr[6].convert(sol);
        }
		//msgpack::type::make_define(__VA_ARGS__).msgpack_unpack(o);
	}

};

class StorageProtocol
{
public:
    virtual ~StorageProtocol() { }
    virtual void createNewGenome(const std::string& name, unsigned length) = 0;
    virtual void insertGenomeData(const std::string& name, unsigned& index, const std::vector<unsigned char>& data) = 0;
    virtual bool insertSolution(const ProblemDescription& prob, const Solution& solution) = 0;
    // The calling function is responsible for deleting the QueryResponse
    virtual QueryResponse * queryByProblemID(const ProblemID& problemID, bool entireSolution) = 0;
    virtual QueryResponse * queryByInitialConditions(const ProblemDescription& problemDescription, bool wantPartials) = 0;

    // query a name, recieve the genome.
    virtual std::vector<unsigned char> queryByName(const std::string& name, int startIndex, int length) = 0;
    
    virtual ProblemID getNextSolutionID() = 0;
    virtual void getGenomeList(std::map<std::string, int>& genome_list) = 0;
};

class StorageProtocolImpl : public StorageProtocol
{
    boost::asio::ip::tcp::iostream& socket;
    
public:
    StorageProtocolImpl(boost::asio::ip::tcp::iostream& s)
    : socket(s)
    { }
    
    virtual void createNewGenome(const std::string& name, unsigned length);
    virtual void insertGenomeData(const std::string& name, unsigned& index, const std::vector<unsigned char>& data);
    virtual bool insertSolution(const ProblemDescription& prob, const Solution& solution);
    virtual QueryResponse* queryByProblemID(const ProblemID& problemID, bool entireSolution);
    virtual QueryResponse* queryByInitialConditions(const ProblemDescription& problemDescription, const bool wantPartials);
    
    virtual std::vector<unsigned char> queryByName(const std::string& name, int startIndex, int length);
    virtual ProblemID getNextSolutionID();
    virtual void getGenomeList(std::map<std::string, int>& genome_list);
};

#endif /* __PROTOCOL_COMMON_H__ */
