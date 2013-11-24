#include "protocol_impl.h"

void WorkerProtocolImpl::requestProblemList(std::vector<ProblemDescription>& problemList)
{
}

bool WorkerProtocolImpl::claimProblems(const std::vector<ProblemID>& problems)
{
}

void WorkerProtocolImpl::sendSolution(const Solution& solution)
{
}

bool StorageProtocolImpl::insertSolution(const Solution& solution)
{
}

bool StorageProtocolImpl::queryProblemID(const ProblemID& problemID)
{
}
