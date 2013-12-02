#include <sys/socket.h>

#include "protocol.h"
#include "problem.h"

void readMatrix(int socket, Matrix& mat, const std::string& err)
{
    int length, width;
    readItem(socket, length, "Error reading length of " + err);
    readItem(socket, width, "Error reading width of " + err);
    mat.resize(length, width);
    
    for( int i = 0; i <= mat.getWidth(); ++i ){
        recvfrom(socket, mat.matrix[i], (mat.getLength() + 1)*sizeof(int), 0, NULL, NULL);
    }
}

void sendMatrix(int socket, const Matrix& mat, const std::string& err)
{
    int length = mat.getLength();
    int width = mat.getWidth();
    sendItem(socket, length, "Error sending length of " + err);
    sendItem(socket, width, "Error sending width of " + err);
    
    for( int i = 0; i <= mat.getWidth(); ++i ) {
        send(socket, mat.matrix[i], (mat.getLength() + 1)*sizeof(int), 0);
    }
}

void readMatrix(std::istream& socket, Matrix& mat, const std::string& err)
{
    int length, width;
    readItem(socket, length, "Error reading length of " + err);
    readItem(socket, width, "Error reading width of " + err);
    mat.resize(length, width);
    
    for( int i = 0; i <= mat.getWidth(); ++i ){
        socket.read(reinterpret_cast<char*>(mat.matrix[i]), (mat.getLength() + 1)*sizeof(int));
    }
}

void sendMatrix(std::ostream& socket, const Matrix& mat, const std::string& err)
{
    int length = mat.getLength();
    int width = mat.getWidth();
    writeItem(socket, length, "Error sending length of " + err);
    writeItem(socket, width, "Error sending width of " + err);
    
    for( int i = 0; i <= mat.getWidth(); ++i ) {
        socket.write(reinterpret_cast<const char*>(mat.matrix[i]), (mat.getLength() + 1)*sizeof(int));
    }
}

void readProblemDescription(std::istream& socket, ProblemDescription& cur_prob)
{
    readItem(socket, cur_prob.problemID.idnum);
    readItem(socket, cur_prob.corner);
    readVector(socket, cur_prob.top_numbers);
    readVector(socket, cur_prob.left_numbers);
    readVector(socket, cur_prob.top_genome);
    readVector(socket, cur_prob.left_genome);
}

void sendProblemDescription(std::ostream& socket, const ProblemDescription& cur_prob, const std::string&)
{
    // TODO use err passed in
    sendItem(socket, cur_prob.problemID.idnum, "Error sending problem ID");
    sendItem(socket, cur_prob.corner, "Error sending problem corner element");
    sendVector(socket, cur_prob.top_numbers, "Error sending the top numbers of a problem");
    sendVector(socket, cur_prob.left_numbers, "Error sending the left numbers of a problem");
    sendVector(socket, cur_prob.top_genome, "Error sending the top genome of a problem");
    sendVector(socket, cur_prob.left_genome, "Error sending the left genome of a problem");
}

void sendProblemDescription(int socket, const ProblemDescription& cur_prob)
{
    sendItem(socket, cur_prob.problemID.idnum, "Error sending problem ID");
    sendItem(socket, cur_prob.corner, "Error sending problem corner element");
    sendVector(socket, cur_prob.top_numbers, "Error sending the top numbers of a problem");
    sendVector(socket, cur_prob.left_numbers, "Error sending the left numbers of a problem");
    sendVector(socket, cur_prob.top_genome, "Error sending the top genome of a problem");
    sendVector(socket, cur_prob.left_genome, "Error sending the left genome of a problem");
}

void readProblemDescription(int socket, ProblemDescription& cur_prob)
{
    readItem(socket, cur_prob.problemID.idnum, "Error reading problem id");
    readItem(socket, cur_prob.corner, "Error reading corner element");
    readVector(socket, cur_prob.top_numbers, "Error reading top numbers");
    readVector(socket, cur_prob.left_numbers, "Error reading left numbers");
    readVector(socket, cur_prob.top_genome, "Error reading top genome");
    readVector(socket, cur_prob.left_genome, "Error reading left genome");
}

void readSolution(int sock, Solution& sol)
{
    readItem(sock, sol.id, "Error reading solution id");
    readItem(sock, sol.maxValue, "Error reading the maximum value in the solution");
    readItem(sock, sol.maxValueLocation, "Error reading the location of the max value");
    readMatrix(sock, sol.matrix, "solution matrix");
}

void sendSolution(int sock, const Solution& sol)
{
    sendItem(sock, sol.id, "Error sending solution id");
    sendItem(sock, sol.maxValue, "Error sending the maximum value in the solution");
    sendItem(sock, sol.maxValueLocation, "Error sending the location of the max value");
    sendMatrix(sock, sol.matrix, "solution matrix");
}

void readSolution(std::istream& sock, Solution& sol, const std::string&)
{
    // TODO use the err passed in
    readItem(sock, sol.id, "Error reading solution id");
    readItem(sock, sol.maxValue, "Error reading the maximum value in the solution");
    readItem(sock, sol.maxValueLocation, "Error reading the location of the max value");
    readMatrix(sock, sol.matrix, "solution matrix");
}

void sendSolution(std::ostream& sock, const Solution& sol, const std::string&)
{
    // TODO use the err passed in
    sendItem(sock, sol.id, "Error sending solution id");
    sendItem(sock, sol.maxValue, "Error sending the maximum value in the solution");
    sendItem(sock, sol.maxValueLocation, "Error sending the location of the max value");
    sendMatrix(sock, sol.matrix, "solution matrix");
}

void sendQueryResponse(int sock, const QueryResponse& resp)
{
    sendItem(sock, resp.success, "Error sending query response success");
    if( !resp.success )
        return;
    
    sendItem(sock, resp.exactMatch, "Error sending query response exact match flag");
    sendProblemDescription(sock, resp.problemDescription);
    sendItem(sock, resp.maxValue, "Error sending query response max value");
    sendItem(sock, resp.location, "Error sending query response max location");
    if( resp.sol ) {
        sendSolution(sock, *resp.sol);
    }
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

void StorageProtocolImpl::insertGenomeData(const std::string& name, unsigned& index, const std::vector<char>& data)
{
    sendItem(socket, static_cast<message_id_t>(STORE_NEW_DATA_ID));
    
    sendString(socket, name); // TODO. Whoever is listening on the other end needs to read the name in as well.
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
