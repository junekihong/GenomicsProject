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
    msgpack::unpacker unpack;
    readBuffer(socket, unpack);
    message_id_t msg_id;
    read(unpack, msg_id, "Error reading message id");
    
    switch( msg_id )
    {
        case PROBLEM_LIST_REQUEST_ID:
#ifdef DEBUG
            std::cout << "WorkerHandler: handleListRequest()\n";
#endif        
            handleListRequest(unpack);
            break;
        case PROBLEM_CLAIM_REQUEST_ID:
#ifdef DEBUG
            std::cout << "WorkerHandler: handleClaimRequest()\n";
#endif
            handleClaimRequest(unpack);
            break;
        case SOLUTION_REPORT_ID:
#ifdef DEBUG
            std::cout << "WorkerHandler: handleSolutionReport()\n";
#endif
            handleSolutionReport(unpack);
            break;
        default:
            throw std::runtime_error(std::string("Unknown message type from worker: ") + toString(msg_id));
    }

    return true;
}

void WorkerHandler::handleListRequest(msgpack::unpacker&)
{
    actions->requestProblemList();
}

void WorkerHandler::handleClaimRequest(msgpack::unpacker& unpack)
{
    std::vector<ProblemID> problems;
    read(unpack, problems);
    
    actions->claimProblems(problems);
}

void WorkerHandler::handleSolutionReport(msgpack::unpacker& unpack)
{
    SolutionCertificate sol;
    read(unpack, sol);
    
    actions->recieveSolution(sol);
}

void WorkerProtocolImpl::sendProblemList(const std::vector<ProblemDescription> &problemList)
{
#ifdef DEBUG
    std::cout << "Sending the list of problems:\n";
    for( unsigned i = 0; i < problemList.size(); ++i )
        std::cout << "\t" << problemList[i].problemID.idnum << "\n";
#endif
    msgpack::sbuffer sbuf;
    message_id_t msg_id = PROBLEM_LIST_RESPONSE_ID;
    msgpack::pack(sbuf, msg_id);
    msgpack::pack(sbuf, problemList);
    sendBuffer(socket, sbuf);
}

void WorkerProtocolImpl::respondToProblemClaim(bool answer)
{
    message_id_t msg_id = PROBLEM_CLAIM_RESPONSE_ID;
    msgpack::sbuffer sbuf;
    msgpack::pack(&sbuf, msg_id);
    msgpack::pack(&sbuf, answer);
    sendBuffer(socket, sbuf);
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
    
    virtual int getSocket() const {
        return socket;
    }
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
        return handleGenomeContinuation();
    }
    
    msgpack::unpacker unpack;
    readBuffer(socket, unpack);
    message_id_t msg_id;
    read(unpack, msg_id, "Error reading message id from client");
    
    switch( msg_id )
    {
        case GENOME_UPLOAD_START_ID:
#ifdef DEBUG
            std::cout << "ClientHandler: handleGenomeStart()\n";
#endif
            handleGenomeStart(unpack);
            return true;
            break;
        case GENOME_LIST_REQUEST_ID:
#ifdef DEBUG
            std::cout << "clientHandler: handleGenomeListRequest()\n";
#endif
            handleGenomeListRequest(unpack);
            return false;
            break;
        case LOCAL_ALIGN_START_ID:
#ifdef DEBUG
            std::cout << "clientHandler: handleAlignmentStart()\n";
#endif
            handleAlignmentStart(unpack);
            return true;
            break;
        default:
            throw std::runtime_error("Got unknown message id from client: " + toString(msg_id));
            return false;
    }
}

void ClientHandler::handleGenomeListRequest(msgpack::unpacker& unpack)
{
    actions->listGenomes();
}

void ClientHandler::handleGenomeStart(msgpack::unpacker& unpack)
{
    std::string name;
    read(unpack, name);
    
    read(unpack, uploadLength, "Error reading genome length from client");
    
    uploadInProgress = true;
    uploadProgress = 0;
    
    actions->startGenomeUpload(name, uploadLength);
}

void ClientHandler::handleGenomeFinish()
{
    actions->finishGenomeUpload();
}

bool ClientHandler::handleGenomeContinuation()
{
    const unsigned buff_len = BUFF_SIZE;
    unsigned cur_buf_len = std::min(buff_len, uploadLength - uploadProgress);
    std::vector<unsigned char> buffer(cur_buf_len);
    ssize_t bytes_read = recvfrom(socket, buffer.data(), buffer.size(), 0, NULL, NULL);
    if( bytes_read < 0 )
    {
        throw std::runtime_error("Error receiving genome data from client: " + toString(bytes_read));
    }
    buffer.resize(bytes_read);

    actions->continueGenomeUpload(uploadProgress, buffer);
    
    uploadProgress += bytes_read;
    if( uploadProgress == uploadLength ) {
        uploadInProgress = false;
        uploadLength = uploadProgress = 0;

        handleGenomeFinish();
        return false;
    }
    else {
        return true;
    }
}

void ClientHandler::handleAlignmentStart(msgpack::unpacker& unpack)
{
    std::string names[2];
    read(unpack, names[0]);
    read(unpack, names[1]);
    
    actions->alignmentRequest(names[0], names[1]);
}

void ClientProtocolImpl::sendGenomeList(const std::vector<std::string> &nameList)
{
    msgpack::sbuffer sbuf;
    message_id_t msg_id = GENOME_LIST_RESPONSE_ID;
    msgpack::pack(&sbuf, msg_id);
    
    msgpack::pack(&sbuf, nameList);
    sendBuffer(socket, sbuf);
}

void ClientProtocolImpl::sendLocalAlignResponse(const Solution& solution)
{
    msgpack::sbuffer sbuf;
    message_id_t msg_id = LOCAL_ALIGN_FINISH_ID;
    msgpack::pack(sbuf, msg_id);
    msgpack::pack(sbuf, solution);
    sendBuffer(socket, sbuf);
}

void ClientProtocolImpl::sendGenomeUploadResponse()
{
    send_ack(socket, UPLOAD_REQUEST_RECIEVED_ID);
}
