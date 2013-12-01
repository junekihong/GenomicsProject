#include <iostream>

#include "common/protocol.h"
#include "protocol_impl.h"

void WorkerProtocolImpl::requestProblemList(std::vector<ProblemDescription>& problemList)
{
	message_id_t msg_id = PROBLEM_LIST_REQUEST_ID;
	
	std::cout << "Requesting problem list";
    sendItem(socket, msg_id);
	
    readItem(socket, msg_id);
	
	if( msg_id != PROBLEM_LIST_RESPONSE_ID )
	{
		std::cout << "Requested problem list, but got back message type " << msg_id << " instead.";
		throw std::runtime_error("Error in protocol talking to leader.  See logs");
	}
	
	unsigned problem_count;
    readItem(socket, problem_count);
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
    message_id_t msg_id = PROBLEM_CLAIM_REQUEST_ID;
    sendItem(socket, msg_id);
    sendItem(socket, static_cast<unsigned>(problems.size()));
    socket.write(reinterpret_cast<const char*>(problems.data()), problems.size() * sizeof(ProblemID));
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

//name is the name of the string
//length is the length of the genome
void StorageProtocolImpl::createNewGenome(const std::string& name, unsigned length)
{
    sendItem(socket, static_cast<message_id_t>(STORE_NEW_GENOME_ID));
    sendString(socket, name);
    sendItem(socket, length);
    socket.flush();    

    message_id_t responseMessage;
    readItem(socket, responseMessage);
    //TODO do something with responseMessage
    // if(responseMessage == STORE_QUERY_RESPONSE_ID) {}
}

void StorageProtocolImpl::insertGenomeData(unsigned index, std::vector<char>& data)
{
    sendItem(socket, index);
    sendVector(socket, data, "Error. Was not able to send data to storage from worker.");
    socket.flush();

    message_id_t responseMessage;
    readItem(socket, responseMessage);
    //TODO do something with responseMessage
    // if(responseMessage == STORE_QUERY_RESPONSE_ID) {}
}

bool StorageProtocolImpl::insertSolution(const Solution& solution)
{
    sendItem(socket, static_cast<message_id_t>(STORE_NEW_SOLUTION_ID));
    sendSolution(socket, solution, "Error. Could not insert solution to storage.");
    socket.flush();
    
    message_id_t responseMessage;
    readItem(socket, responseMessage);
    return responseMessage == STORE_QUERY_RESPONSE_ID;
}

QueryResponse* StorageProtocolImpl::queryByProblemID(const ProblemID& problemID, bool entireSolution)
{
    sendItem(socket, static_cast<message_id_t>(STORE_QUERY_BY_ID_ID));
    sendItem(socket, problemID.idnum);
    sendItem(socket, entireSolution);
    socket.flush();

    message_id_t responseMessage;
    readItem(socket, responseMessage);
    if(responseMessage != STORE_QUERY_RESPONSE_ID)
    {
        std::cerr << "Error. Storage did not properly respond to our query by problemID\n";
        return NULL;
    }
    
    QueryResponse* response = new QueryResponse();
    readItem(socket, *(response), "Error. Failed to read the response to the query by problemID");
    
	return response;
}

QueryResponse* StorageProtocolImpl::queryByInitialConditions(const ProblemDescription& problemDescription, const bool wantPartials)
{
    sendItem(socket, static_cast<message_id_t>(STORE_QUERY_BY_COND_ID));
    sendProblemDescription(socket, problemDescription, "Error. Could not send a query by initial conditions");
    sendItem(socket, wantPartials);
    socket.flush();

    message_id_t responseMessage;
    readItem(socket, responseMessage);
    if(responseMessage != STORE_QUERY_RESPONSE_ID)
    {
        std::cerr << "Error. Storage did not properly respond to our query by initial conditians\n";
        return NULL;
    }

    QueryResponse* response = new QueryResponse();
    readItem(socket, *(response), "Error. Failed to read the response to the query by initial conditions");

    return response;
}


