#include <iostream>

#include "protocol_impl.h"

#define PROBLEM_LIST_REQUEST_ID		3
#define PROBLEM_LIST_RESPONSE_ID	3

typedef int message_id_t;

void WorkerProtocolImpl::requestProblemList(std::vector<ProblemDescription>& problemList)
{
	message_id_t msg_id = PROBLEM_LIST_REQUEST_ID;
	
	std::cout << "Requesting problem list";
	socket << msg_id;
    socket.flush();
	
	socket >> msg_id;
	
	if( msg_id != PROBLEM_LIST_RESPONSE_ID )
	{
		std::cout << "Requested problem list, but got back message type " << msg_id << " instead.";
		throw std::runtime_error("Error in protocol talking to leader.  See logs");
	}
	
	unsigned problem_count;
	socket >> problem_count;
    std::cout << "Receiving " << problem_count << " problems.\n";
	problemList.resize(problem_count);
	for( unsigned prob_idx = 0; prob_idx < problem_count; ++ prob_idx )
	{
        ProblemDescription &cur_prob = problemList[prob_idx];
        socket >> cur_prob.id.idnum;
        
        unsigned vec_length;
        socket >> vec_length;
        cur_prob.top_numbers.resize(vec_length);
        socket.read(reinterpret_cast<char*>(cur_prob.top_numbers.data()), vec_length * sizeof(std::vector<int>::value_type));
        cur_prob.top_genome.resize(vec_length);
        socket.read(reinterpret_cast<char*>(cur_prob.top_genome.data()), vec_length * sizeof(std::vector<char>::value_type));

        socket >> vec_length;
        cur_prob.left_numbers.resize(vec_length);
        socket.read(reinterpret_cast<char*>(cur_prob.left_numbers.data()), vec_length * sizeof(std::vector<int>::value_type));
        cur_prob.left_genome.resize(vec_length);
        socket.read(reinterpret_cast<char*>(cur_prob.left_genome.data()), vec_length * sizeof(std::vector<char>::value_type));
    }
    std::cout << "Finished receving problem list\n";
}

bool WorkerProtocolImpl::claimProblems(const std::vector<ProblemID>& problems)
{
	return false;
}

void WorkerProtocolImpl::sendSolution(const Solution& solution)
{
}

bool StorageProtocolImpl::insertSolution(const Solution& solution)
{
	return false;
}

bool StorageProtocolImpl::queryProblemID(const ProblemID& problemID)
{
	return false;
}
