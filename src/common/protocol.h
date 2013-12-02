#ifndef __PROTOCOL_COMMON_H__
#define __PROTOCOL_COMMON_H__

#include <iostream>
#include <stdexcept>

#include <boost/asio/ip/tcp.hpp>

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

#define STORE_NEW_GENOME_ID             12
#define STORE_NEW_DATA_ID                   13
#define STORE_NEW_SOLUTION_ID               14
#define STORE_QUERY_BY_ID_ID                15
#define STORE_QUERY_BY_COND_ID              16

#define STORE_GENOME_INFO_QUERY_ID          17
#define STORE_GENOME_INFO_RESPONSE_ID       17

#define STORE_GENOME_CONTENT_QUERY_ID       18
#define STORE_GENOME_CONTENT_RESPONSE_ID    18

#define STORE_QUERY_RESPONSE_ID             19

#define BUFF_SIZE 32*1024

class ProblemDescription;

typedef int message_id_t;

void readMatrix(int socket, Matrix& mat, const std::string& err);
void sendMatrix(int socket, const Matrix& mat, const std::string& err);

void readProblemDescription(std::istream& socket, ProblemDescription& cur_prob);
void readProblemDescription(int socket, ProblemDescription& cur_prob);
void sendProblemDescription(int socket, const ProblemDescription& cur_prob);
void sendProblemDescription(std::ostream& socket, const ProblemDescription& cur_prob, const std::string& err);

void readSolution(int sock, Solution& sol);
void sendSolution(int sock, const Solution& sol);
void readSolution(std::istream& sock, Solution& sol, const std::string& err);
void sendSolution(std::ostream& sock, const Solution& sol, const std::string& err);

class QueryResponse;
void sendQueryResponse(int sock, const QueryResponse& resp);

template<typename T>
static inline void readItem(std::istream& socket, T& dest, const std::string& err = "")
{
    socket.read(reinterpret_cast<char*>(&dest), sizeof(dest));
    if( !socket )
        throw std::runtime_error(err);
}
template<typename T>
static inline void sendItem(std::ostream& socket, const T& dest, const std::string& err = "")
{
    socket.write(reinterpret_cast<const char*>(&dest), sizeof(dest));
    if( !socket )
        throw std::runtime_error(err);
}

/* When I tried to use sendItem in sendMatrix(std::ostream&...), the compiler
 * could not resolve which template I wanted. So writeItem is here. */
template<typename T>
static inline void writeItem(std::ostream& socket, const T& dest, const std::string& err = "")
{
    socket.write(reinterpret_cast<const char*>(&dest), sizeof(dest));
    if( !socket )
        throw std::runtime_error(err);
}

static inline std::string readString(std::istream& sock)
{
    unsigned length;
    readItem(sock, length);
    char * buff = new char[length + 1]; // TODO leaks on exceptions
    sock.read(buff, length);
    buff[length] = 0;
    std::string result(buff);
    delete []buff;
    return result;
}

static inline void sendString(std::ostream& sock, const std::string& str)
{
    unsigned length = static_cast<unsigned>(str.size()); // TODO loses precision
    sendItem(sock, length);
    sock.write(str.data(), str.size());
}

template<typename T>
static inline void readVector(std::istream& sock, std::vector<T>& result)
{
    unsigned length;
    readItem(sock, length);
    result.resize(length);
    sock.read(reinterpret_cast<char*>(result.data()), length *sizeof(T));
}

template<typename T>
static inline void sendVector(std::ostream& sock, const std::vector<T>& vec, const std::string& err)
{
    unsigned length = static_cast<unsigned>(vec.size()); // TODO loses precision
    sendItem(sock, length);
    sock.write(reinterpret_cast<const char*>(vec.data()), vec.size() * sizeof(T));
    if( !sock )
        throw std::runtime_error(err);
}

#ifdef MSG_WAITALL

// To use this function, include sys/socket.h before including this file
template<typename T>
static inline void readItem(int socket, T& item, const std::string& err_message)
{
    ssize_t bytes_read = recvfrom(socket, &item, sizeof(item), MSG_WAITALL, NULL, NULL);
    if( bytes_read != sizeof(item) )
    {
        throw std::runtime_error(err_message);
    }
}

template<typename T>
static inline void sendItem(int socket, T& item, const std::string& err_message)
{
    ssize_t bytes_sent = send(socket, &item, sizeof(item), 0);
    if( bytes_sent != sizeof(item) )
    {
        throw std::runtime_error(err_message);
    }
}

static inline std::string readString(int sock, const std::string& err)
{
    unsigned length;
    readItem(sock, length, "Error reading the length of " + err);
    char * buff = new char[length + 1]; // TODO leaks on exceptions
    ssize_t bytes_read = recvfrom(sock, buff, length, MSG_WAITALL, NULL, NULL);
    if( bytes_read != length )
        throw std::runtime_error("Error reading " + err);
    buff[length] = 0;
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
static inline void readVector(int sock, std::vector<T>& result, const std::string& err)
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
    if( bytes_sent != length * sizeof(T) )
        throw std::runtime_error("Error sending " + err);
}

#endif

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
    
    ~QueryResponse()
    {
        delete sol;
    }
    
    // TODO figure out a way to send back solution certificates
};

class StorageProtocol
{
public:
    virtual void createNewGenome(const std::string& name, unsigned length) = 0;
    virtual void insertGenomeData(const std::string& name, unsigned& index, const std::vector<char>& data) = 0;
    virtual bool insertSolution(const Solution& solution) = 0;
    // The calling function is responsible for deleting the QueryResponse
    virtual QueryResponse * queryByProblemID(const ProblemID& problemID, bool entireSolution) = 0;
    virtual QueryResponse * queryByInitialConditions(const ProblemDescription& problemDescription, bool wantPartials) = 0;
    
};

class StorageProtocolImpl : public StorageProtocol
{
    boost::asio::ip::tcp::iostream& socket;
    
public:
    StorageProtocolImpl(boost::asio::ip::tcp::iostream& s)
    : socket(s)
    { }
    
    virtual void createNewGenome(const std::string& name, unsigned length);
    virtual void insertGenomeData(const std::string& name, unsigned& index, const std::vector<char>& data);
    virtual bool insertSolution(const Solution& solution);
    virtual QueryResponse* queryByProblemID(const ProblemID& problemID, bool entireSolution);
    virtual QueryResponse* queryByInitialConditions(const ProblemDescription& problemDescription, const bool wantPartials);
    
};

#endif /* __PROTOCOL_COMMON_H__ */
