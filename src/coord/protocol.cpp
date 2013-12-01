#include <stdexcept>
#include <sstream>

#include <sys/types.h>
#include <sys/socket.h>

#include "common/protocol.h"
#include "common/util.h"
#include "protocol.h"

class WorkerProtocolImpl : public LeaderWorkerProtocol
{
    int socket;
    public:
    WorkerProtocolImpl(int s)
        : socket(s)
    { }
    
    virtual void sendProblemList(const std::vector<ProblemDescription>& problemList);
    virtual void respondToProblemClaim(bool answer);
};

WorkerHandler::WorkerHandler(int socket)
: NetworkHandler(socket), actions(NULL)
{
    LeaderWorkerProtocol * w = new WorkerProtocolImpl(socket);
    actions = workerActionFactory(w);
}

bool WorkerHandler::handleNetwork()
{
    message_id_t msg_id;
    
    readItem(socket, msg_id, "Error reading message id");
    
    switch( msg_id )
    {
        case PROBLEM_LIST_REQUEST_ID:
            handleListRequest();
            break;
        case PROBLEM_CLAIM_REQUEST_ID:
            handleClaimRequest();
            break;
        case SOLUTION_REPORT_ID:
            handleSolutionReport();
            break;
        default:
            throw std::runtime_error(std::string("Unknown message type from worker: ") + toString(msg_id));
    }

    return true;
}

void WorkerHandler::handleListRequest()
{
    actions->requestProblemList();
}

void WorkerHandler::handleClaimRequest()
{
    unsigned claim_count;
    readItem(socket, claim_count, "Error reading problem claim count");
    std::vector<ProblemID> problems(claim_count);
    for( unsigned i = 0; i < claim_count; ++i )
    {
        readItem(socket, problems[i], "Error reading problem id number " + toString(i));
    }
    
    actions->claimProblems(problems);
}

void WorkerHandler::handleSolutionReport()
{
    SolutionCertificate sol;
    readItem(socket, sol.problemID, "Error reading solution problem id");
    readItem(socket, sol.solutionID, "Error reading solution solution id");
    
    actions->recieveSolution(sol);
}

void WorkerProtocolImpl::sendProblemList(const std::vector<ProblemDescription> &problemList)
{
    message_id_t msg_id = PROBLEM_LIST_RESPONSE_ID;
    // TOOD error checks
    send(socket, &msg_id, sizeof(msg_id), 0);
    unsigned problem_count = static_cast<unsigned>(problemList.size());
    send(socket, &problem_count, sizeof(problem_count), 0);
    for( unsigned i = 0; i < problem_count; ++i )
    {
        sendProblemDescription(socket, problemList[i]);
    }
}

void WorkerProtocolImpl::respondToProblemClaim(bool answer)
{
    message_id_t msg_id = PROBLEM_CLAIM_RESPONSE_ID;
    send(socket, &msg_id, sizeof(msg_id), 0);
    send(socket, &answer, sizeof(answer), 0);
}

/* Client classes */
class ClientProtocolImpl : public LeaderClientProtocol
{
    int socket;
    
    public:
    ClientProtocolImpl(int s)
        : socket(s)
    { }
    
    virtual void sendGenomeList(const std::vector<std::string>& nameList);
    virtual void sendLocalAlignResponse(const Solution& solution);

    virtual void sendGenomeUploadResponse();
};

ClientHandler::ClientHandler(int socket)
    : NetworkHandler(socket), uploadInProgress(false), uploadProgress(0), uploadLength(0)
{
    LeaderClientProtocol * c = new ClientProtocolImpl(socket);
    actions = clientActionFactory(c);
}

bool ClientHandler::handleNetwork()
{
    if( uploadInProgress )
    {
        handleGenomeContinuation();
        return true;
    }
    
    message_id_t msg_id;
    readItem(socket, msg_id, "Error reading message id from client");
    
    switch( msg_id )
    {
        case GENOME_UPLOAD_START_ID:
            handleGenomeStart();
            return false;
            break;
        case GENOME_LIST_REQUEST_ID:
            handleGenomeListRequest();
            return false;
            break;
        case LOCAL_ALIGN_START_ID:
            handleAlignmentStart();
            return true;
            break;
        default:
            throw std::runtime_error("Got unknown message id from client: " + toString(msg_id));
            return false;
    }
}

void ClientHandler::handleGenomeListRequest()
{
    actions->listGenomes();
}

void ClientHandler::handleGenomeStart()
{
    unsigned name_length;
    char * name_buffer;
    
    readItem(socket, name_length, "Error reading genome length from client");
    name_buffer = new char[name_length + 1]; // TODO leaks when exceptions get thrown
    recvfrom(socket, name_buffer, name_length, MSG_WAITALL, NULL, NULL); // TODO error check
    name_buffer[name_length] = 0;
    std::string name = name_buffer;
    delete[] name_buffer;
    
    readItem(socket, uploadLength, "Error reading genome length from client");
    
    uploadInProgress = true;
    uploadProgress = 0;
    
    actions->startGenomeUpload(name_buffer, uploadLength);
}

void ClientHandler::handleGenomeFinish()
{
    actions->finishGenomeUpload();
}

void ClientHandler::handleGenomeContinuation()
{
    const unsigned buff_len = 32*1024;
    unsigned cur_buf_len = std::min(buff_len, uploadLength - uploadProgress);
    std::vector<char> buffer(cur_buf_len);
    ssize_t bytes_read = recvfrom(socket, buffer.data(), buffer.size(), 0, NULL, NULL);
    if( bytes_read < 0 )
    {
        throw std::runtime_error("Error receiving genome data from client: " + toString(bytes_read));
    }
    buffer.resize(bytes_read);
    actions->continueGenomeUpload(buffer);
    
    uploadProgress += bytes_read;
    if( uploadProgress == uploadLength ) {
        uploadInProgress = false;
        uploadLength = uploadProgress = 0;

        handleGenomeFinish();
    }
}

void ClientHandler::handleAlignmentStart()
{
    unsigned string_len = 0;
    
    readItem(socket, string_len, "Error reading local alignment genome name length");
    char * buffer = new char[string_len + 1];
    ssize_t bytes_read = recvfrom(socket, buffer, string_len, MSG_WAITALL, NULL, NULL);
    if( bytes_read < 0 )
    {
        throw std::runtime_error("Error reading local alignment genome name");
    }
    buffer[string_len] = 0;
    std::string names[2];
    names[0] = buffer;
    delete[] buffer;
    
    readItem(socket, string_len, "Error reading local alignment genome name length");
    buffer = new char[string_len + 1];
    bytes_read = recvfrom(socket, buffer, string_len, MSG_WAITALL, NULL, NULL);
    if( bytes_read < 0 )
    {
        throw std::runtime_error("Error reading local alignment genome name");
    }
    buffer[string_len] = 0;
    names[1] = buffer;
    delete[] buffer;
    
    actions->alignmentRequest(names[0], names[1]);
}

void ClientProtocolImpl::sendGenomeList(const std::vector<std::string> &nameList)
{
    message_id_t msg_id = GENOME_LIST_RESPONSE_ID;
    sendItem(socket, msg_id, "Error sending Genome List response message id");
    
    unsigned length = static_cast<unsigned>(nameList.size());
    sendItem(socket, length, "Error sending the number of genomes to the client");
    for( unsigned i = 0; i < nameList.size(); ++i )
    {
        length = static_cast<unsigned>(nameList[i].size());
        sendItem(socket, length, "Error sending the length of a genome name to the client");
        send(socket, nameList[i].data(), nameList[i].size(), 0);
    }
}

void ClientProtocolImpl::sendLocalAlignResponse(const Solution& solution)
{
    message_id_t msg_id = LOCAL_ALIGN_FINISH_ID;
    sendItem(socket, msg_id, "Error sending local align response id");
    sendSolution(socket, solution);
}

void ClientProtocolImpl::sendGenomeUploadResponse()
{
    message_id_t msg_id = UPLOAD_REQUEST_RECIEVED_ID;
    sendItem(socket, msg_id, "Error sending message back to the client that the upload request was recieved");
    
    
}
