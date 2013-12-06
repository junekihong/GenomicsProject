#include <iostream>

#include "common/protocol.h"
#include "protocol_impl.h"

void WorkerProtocolImpl::requestProblemList(std::vector<ProblemDescription>& problemList)
{
#ifdef DEBUG
	std::cout << "Requesting problem list\n";
#endif
    send_ack(socket, PROBLEM_LIST_REQUEST_ID);
	
    msgpack::unpacker unpack;
    readBuffer(socket, unpack);
    
    message_id_t msg_id;
    read(unpack, msg_id);
	
	if( msg_id != PROBLEM_LIST_RESPONSE_ID )
	{
		std::cout << "Requested problem list, but got back message type " << msg_id << " instead.";
		throw std::runtime_error("Error in protocol talking to leader.  See logs");
	}
	
    read(unpack, problemList);
}

bool WorkerProtocolImpl::claimProblems(const std::vector<ProblemID>& problems)
{
#ifdef DEBUG
    std::cout << "Requesting " << problems.size() << " problems\n";
    std::cout << "They are:\n";
    for( unsigned i = 0; i < problems.size(); ++i )
        std::cout << "\t" << problems[i].idnum << "\n";
#endif
    msgpack::sbuffer sbuf;
    message_id_t msg_id = PROBLEM_CLAIM_REQUEST_ID;
    msgpack::pack(&sbuf, msg_id);
    msgpack::pack(&sbuf, problems);
    sendBuffer(socket, sbuf);
    
    msgpack::unpacker unpack;
    readBuffer(socket, unpack);
    read(unpack, msg_id);
	if( msg_id != PROBLEM_CLAIM_RESPONSE_ID )
	{
		std::cout << "Attempted to claim problems, but got back message type " << msg_id << " instead.";
		throw std::runtime_error("Error in protocol talking to leader.  See logs");
	}
	
    bool result;
    read(unpack, result);
    
	return result;
}

void WorkerProtocolImpl::sendSolution(const SolutionCertificate& solution)
{
    msgpack::sbuffer sbuf;
    msgpack::pack(&sbuf, static_cast<message_id_t>(SOLUTION_REPORT_ID));
    msgpack::pack(&sbuf, solution);
    sendBuffer(socket, sbuf);
}
