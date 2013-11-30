#include <stdlib.h>
#include "protocol.h"


class WorkerActionImpl : public WorkerActions
{
    LeaderWorkerProtocol * worker;
    
    public:
    WorkerActionImpl(LeaderWorkerProtocol * w);
    
    virtual void requestProblemList();
    virtual void claimProblems(const std::vector<ProblemID>& problems);
    virtual void recieveSolution(const SolutionCertificate& solution);
};

WorkerActions * workerActionFactory(LeaderWorkerProtocol * w)
{
    //return NULL;
    return new WorkerActionImpl(w);
}

WorkerActionImpl::WorkerActionImpl(LeaderWorkerProtocol * w)
{
}

void WorkerActionImpl::requestProblemList()
{
}
void WorkerActionImpl::claimProblems(const std::vector<ProblemID>& problems)
{
    std::vector<ProblemID> p = problems;
}

void WorkerActionImpl::recieveSolution(const SolutionCertificate& solution)
{
    SolutionCertificate s = solution;
}


class ClientActionImpl : public ClientActions
{
    LeaderClientProtocol * client;
    
    public:
    ClientActionImpl(LeaderClientProtocol * c);
    
    virtual void startGenomeUpload(const std::string& name, unsigned length);
    virtual void continueGenomeUpload(const std::vector<char>& data);
    virtual void finishGenomeUpload();

    virtual void listGenomes();
    virtual void alignmentRequest(const std::string& first, const std::string& second);
};

ClientActions * clientActionFactory(LeaderClientProtocol * c)
{
    return new ClientActionImpl(c);
}

//TODO
ClientActionImpl::ClientActionImpl(LeaderClientProtocol * c)
{
    client = c;
}

//TODO
void ClientActionImpl::startGenomeUpload(const std::string &name, unsigned int length)
{
    //TODO contact storage
}

//TODO
void ClientActionImpl::continueGenomeUpload(const std::vector<char>& data)
{   
    //TODO contact storage
}

void ClientActionImpl::finishGenomeUpload()
{
    client->sendGenomeUploadResponse();
}

//TODO
void ClientActionImpl::listGenomes()
{

}

//TODO
void ClientActionImpl::alignmentRequest(const std::string& first, const std::string& second)
{
    
    //client->sendLocalAlignResponse();
}

