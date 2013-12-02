#include <iostream>

#include "common/protocol.h"
#include "protocol_impl.h"

void WorkerProtocolImpl::requestProblemList(std::vector<ProblemDescription>& problemList)
{
	message_id_t msg_id = PROBLEM_LIST_REQUEST_ID;
	
#ifdef DEBUG
	std::cout << "Requesting problem list\n";
#endif
    sendItem(socket, msg_id);
	
    readItem(socket, msg_id);
	
	if( msg_id != PROBLEM_LIST_RESPONSE_ID )
	{
		std::cout << "Requested problem list, but got back message type " << msg_id << " instead.";
		throw std::runtime_error("Error in protocol talking to leader.  See logs");
	}
	
	unsigned problem_count;
    readItem(socket, problem_count);
#ifdef DEBUG
    std::cout << "Receiving " << problem_count << " problems.\n";
#endif
	problemList.resize(problem_count);
	for( unsigned prob_idx = 0; prob_idx < problem_count; ++ prob_idx )
	{
        readProblemDescription(socket, problemList[prob_idx]);
    }
    std::cout << "Finished receving problem list\n";
}

bool WorkerProtocolImpl::claimProblems(const std::vector<ProblemID>& problems)
{
#ifdef DEBUG
    std::cout << "Requesting " << problems.size() << " problems\n";
    std::cout << "They are:\n";
    for( unsigned i = 0; i < problems.size(); ++i )
        std::cout << "\t" << problems[i].idnum << "\n";
#endif
    message_id_t msg_id = PROBLEM_CLAIM_REQUEST_ID;
    sendItem(socket, msg_id);
    sendVector(socket, problems, "the vector of problem ids to claim");
    socket.flush();
    
    readItem(socket, msg_id);
	if( msg_id != PROBLEM_CLAIM_RESPONSE_ID )
	{
		std::cout << "Attempted to claim problems, but got back message type " << msg_id << " instead.";
		throw std::runtime_error("Error in protocol talking to leader.  See logs");
	}
	
    bool result;
    readItem(socket, result);
    
	return result;
}

void WorkerProtocolImpl::sendSolution(const SolutionCertificate& solution)
{
    sendItem(socket, static_cast<message_id_t>(SOLUTION_REPORT_ID));
    sendItem(socket, solution.problemID.idnum);
    sendItem(socket, solution.solutionID.idnum);
    socket.flush();
}


