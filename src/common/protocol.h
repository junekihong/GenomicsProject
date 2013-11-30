#ifndef __PROTOCOL_COMMON_H__
#define __PROTOCOL_COMMON_H__

#include <iostream>
#include <stdexcept>

#include "location.h"
#include "problem.h"
#include "solution.h"

#define ANNOUNCE_CLIENT             1
#define ANNOUNCE_LEADER             2
#define ANNOUNCE_WORKER             3
#define ANNOUNCE_STORAGE            4

#define PROBLEM_LIST_REQUEST_ID		5
#define PROBLEM_LIST_RESPONSE_ID	5

#define PROBLEM_CLAIM_REQUEST_ID    6
#define PROBLEM_CLAIM_RESPONSE_ID   6

#define SOLUTION_REPORT_ID          7

#define GENOME_UPLOAD_START_ID      8
#define UPLOAD_REQUEST_RECIEVED_ID  8

#define GENOME_LIST_REQUEST_ID      9
#define GENOME_LIST_RESPONSE_ID     9

#define LOCAL_ALIGN_START_ID        10
#define LOCAL_ALIGN_FINISH_ID       11

#define STORE_NEW_GENOME_ID         12
#define STORE_NEW_DATA_ID           13
#define STORE_NEW_SOLUTION_ID       14
#define STORE_QUERY_BY_ID_ID        15
#define STORE_QUERY_BY_COND_ID      16

#define STORE_GENOME_INFO_QUERY_ID      17
#define STORE_GENOME_INFO_RESPONSE_ID   17

#define STORE_GENOME_CONTENT_QUERY_ID       18
#define STORE_GENOME_CONTENT_RESPONSE_ID    18

#define BUFF_SIZE 32*1024

class ProblemDescription;

typedef int message_id_t;

void readProblemDescription(std::istream& socket, ProblemDescription& cur_prob);

template<typename T>
static inline void readItem(std::istream& socket, T& dest)
{
    socket.read(reinterpret_cast<char*>(&dest), sizeof(dest));
}
template<typename T>
static inline void writeItem(std::ostream& socket, const T& dest)
{
    socket.write(reinterpret_cast<const char*>(&dest), sizeof(dest));
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

#endif

class QueryResponse
{
public:
    int queryResponseID;
    bool potentialMatch;
    bool exactMatch;
    
    // Given problem description.
    ProblemDescription problemDescription;
    
    // Solution description. Maximum value and location in the matrix.
    int maxValue;
    Location location;
    
    // The solution.
    SolutionCertificate solutionCertificate;
};

class StorageProtocol
{
public:
    virtual void createNewGenome(const std::string& name, unsigned length) = 0;
    virtual void insertGenomeData(unsigned index, std::vector<char>& data) = 0;
    virtual bool insertSolution(const Solution& solution) = 0;
    virtual QueryResponse queryByProblemID(const ProblemID& problemID) = 0;
    virtual QueryResponse queryByInitialConditions(const int requestID, const ProblemDescription& problemDescription, const bool queryFlag) = 0;
    
};

#endif /* __PROTOCOL_COMMON_H__ */
