#ifndef __WORKER_PROTOCOL_H__
#define __WORKER_PROTOCOL_H__

#include <vector>

#include "common/problem.h"
#include "common/solution.h"
#include "common/protocol.h"

class WorkerLeaderProtocol
{
public:
    /**
     * \param[out] problemList the list of available problems from the leader
     */
    virtual void requestProblemList(std::vector<ProblemDescription>& problemList) = 0;
    
    /** Attempts to claim the specified problems.
     * \param[in] the list of problems to claim
     * \return whether our claim was successful
     */
    virtual bool claimProblems(const std::vector<ProblemID>& problems) = 0;
    
    virtual void sendSolution(const SolutionCertificate& solution) = 0;
    
};

#endif // __WORKER_PROTOCOL_H__
