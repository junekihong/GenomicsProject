#include <iostream>

#include "common/protocol.h"
#include "protocol_impl.h"

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
        readProblemDescription(socket, problemList[prob_idx]);
    }
    std::cout << "Finished receving problem list\n";
}

bool WorkerProtocolImpl::claimProblems(const std::vector<ProblemID>& problems)
{
    std::cout << "Requesting " << problems.size() << " problems\n";
    socket << static_cast<message_id_t>(PROBLEM_CLAIM_REQUEST_ID);
    socket << static_cast<unsigned>(problems.size());
    socket.write(reinterpret_cast<const char*>(problems.data()), problems.size() * sizeof(ProblemID));
    socket.flush();
    
    message_id_t msg_id = PROBLEM_CLAIM_RESPONSE_ID;
    socket >> msg_id;
	if( msg_id != PROBLEM_CLAIM_RESPONSE_ID )
	{
		std::cout << "Attempted to claim problems, but got back message type " << msg_id << " instead.";
		throw std::runtime_error("Error in protocol talking to leader.  See logs");
	}
	
    bool result;
    socket >> result;
    
	return result;
}

void WorkerProtocolImpl::sendSolution(const SolutionCertificate& solution)
{
    socket << static_cast<message_id_t>(SOLUTION_REPORT_ID);
    socket << solution.problemID.idnum;
    socket << solution.solutionID.idnum;
    socket.flush();
}

bool StorageProtocolImpl::insertSolution(const Solution& solution)
{
	return false;
}

QueryResponse StorageProtocolImpl::queryByProblemID(const ProblemID& problemID)
{
    QueryResponse response;
	return response;
}

QueryResponse StorageProtocolImpl::queryByInitialConditions(const int requestID, const ProblemDescription& problemDescription, const bool queryFlag)
{
    QueryResponse response;
    return response;

}




