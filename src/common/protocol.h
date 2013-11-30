#ifndef __PROTOCOL_COMMON_H__
#define __PROTOCOL_COMMON_H__

#include <iostream>

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

#define GENOME_LIST_REQUEST_ID      9
#define GENOME_LIST_RESPONSE_ID     9

#define LOCAL_ALIGN_START_ID        10
#define LOCAL_ALIGN_FINISH_ID       11

#define UPLOAD_REQUEST_RECIEVED_ID  8


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
