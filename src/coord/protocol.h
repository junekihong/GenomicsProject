#ifndef __LEADER_PROTOCOL_H__
#define __LEADER_PROTOCOL_H__

#include <vector>

#include "common/problem.h"
#include "common/solution.h"

class LeaderWorkerProtocol
{
    public:
    virtual void sendProblemList(const std::vector<ProblemDescription>& problemList) = 0;
    virtual void respondToProblemClaim(bool answer) = 0;
};

class WorkerActions
{
    protected:
    LeaderWorkerProtocol * worker;
    public:
    WorkerActions(LeaderWorkerProtocol * w)
        : worker(w)
    { }
    virtual void requestProblemList() = 0;
    virtual void claimProblems(const std::vector<ProblemID>& problems) = 0;
    virtual void receiveSolution(const SolutionCertificate& solution) = 0;
};

WorkerActions * workerActionFactory(LeaderWorkerProtocol * w);

class NetworkHandler
{
    protected:
    int socket;
    
    public:
    NetworkHandler(int _socket)
        : socket(_socket)
    { }
    virtual ~NetworkHandler() { }
    
    /**
     * Returns false if the network handler should be removed and deleted.
     * The handler should not close the socket.
     */
    virtual bool handleNetwork() = 0;
    
    int getSocket() const
    {
        return socket;
    }
};

class WorkerHandler : public NetworkHandler
{
    WorkerActions * actions;
    public:
    WorkerHandler(int socket);
    
    virtual bool handleNetwork();
    
    private:
    void handleListRequest();
    void handleClaimRequest();
    void handleSolutionReport();
};

#endif /* __LEADER_PROTOCOL_H__ */
