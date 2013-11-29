#include "protocol.h"

#include <stdlib.h>

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
    return true;
}

void WorkerProtocolImpl::sendProblemList(const std::vector<ProblemDescription> &problemList)
{
    
}

void WorkerProtocolImpl::respondToProblemClaim(bool answer)
{
    
}
