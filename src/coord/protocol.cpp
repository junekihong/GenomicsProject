#include <stdexcept>
#include <sstream>

#include <sys/types.h>
#include <sys/socket.h>

#include "common/protocol.h"
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

template<typename T>
static inline std::string toString(T obj)
{
    std::ostringstream strm;
    strm << obj;
    return strm.str();
}

template<typename T>
static inline void readItem(int socket, T& item, const std::string& err_message)
{
    ssize_t bytes_read = recvfrom(socket, &item, sizeof(item), MSG_WAITALL, NULL, NULL);
    if( bytes_read != sizeof(item) )
    {
        throw std::runtime_error(err_message);
    }
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
    
    actions->receiveSolution(sol);
}

void WorkerProtocolImpl::sendProblemList(const std::vector<ProblemDescription> &problemList)
{
    message_id_t msg_id = PROBLEM_LIST_RESPONSE_ID;
    send(socket, &msg_id, sizeof(msg_id), 0);
    unsigned problem_count = static_cast<unsigned>(problemList.size());
    send(socket, &problem_count, sizeof(problem_count), 0);
    send(socket, problemList.data(), problemList.size() * sizeof(ProblemID), 0);
}

void WorkerProtocolImpl::respondToProblemClaim(bool answer)
{
    message_id_t msg_id = PROBLEM_CLAIM_RESPONSE_ID;
    send(socket, &msg_id, sizeof(msg_id), 0);
    send(socket, &answer, sizeof(answer), 0);
}
