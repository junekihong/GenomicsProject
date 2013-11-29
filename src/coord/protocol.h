#ifndef __LEADER_PROTOCOL_H__
#define __LEADER_PROTOCOL_H__

#include <vector>

#include "common/problem.h"

class LeaderWorkerProtocol
{
    public:
    virtual void sendProblemList(std::vector<ProblemDescription>& problemList) = 0;
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
    virtual void receiveSolution(const SolutionCertificat& solution) = 0;
};

#endif /* __LEADER_PROTOCOL_H__ */
