#include <stdlib.h>
#include "protocol.h"
#include "scheduling.h"

std::vector<scheduler::Problem> problemList;
std::vector<std::string> nameList;
int problemNumber = 0;

std::vector<ProblemID> solvedList;


class WorkerActionImpl : public WorkerActions
{    
    public:
    WorkerActionImpl(LeaderWorkerProtocol * w);
    
    virtual void requestProblemList();
    virtual void claimProblems(const std::vector<ProblemID>& problems);
    virtual void recieveSolution(const SolutionCertificate& solution);
};

WorkerActions * workerActionFactory(LeaderWorkerProtocol * w)
{
    return new WorkerActionImpl(w);
}

WorkerActionImpl::WorkerActionImpl(LeaderWorkerProtocol * w)
    : WorkerActions(w)
{
}

void WorkerActionImpl::requestProblemList()
{
    // TODO reconcile how problemList is a vector of scheduler::Problem, wheras sendProblemList expects a vector of ProblemDescriptions
    //worker->sendProblemList(problemList);
}

void WorkerActionImpl::claimProblems(const std::vector<ProblemID>& problems)
{
    for(int i = 0; i < problems.size(); i++)
    {
        // if problem is in problemList continue, else return false.
        bool found = false;
        for(int j = 0; j < problemList.size(); j++)
        {
            if(problemList[j].problemID == problems[i])
            {               
                found = true;
                break;               
            }
        }
        if(!found){
            worker->respondToProblemClaim(false);
            return;
        }
    }
 
    // If we make it here, then all the claimed problems appeared in our problem list. Remove them from the problem list and respond with true.
    for(int i = 0; i < problems.size(); i++)
    {
        std::vector<scheduler::Problem>::iterator iter = problemList.begin();
        while(iter != problemList.end())
        {
            if((*iter).problemID == problems[i]){
                iter = problemList.erase(iter);
            }
            else{
                iter++;
            }
        }
    } 
    worker->respondToProblemClaim(true);
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

ClientActionImpl::ClientActionImpl(LeaderClientProtocol * c)
{
    client = c;
}

//TODO
void ClientActionImpl::startGenomeUpload(const std::string &name, unsigned int length)
{
    //TODO contact storage

    nameList.push_back(name);
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


void ClientActionImpl::listGenomes()
{
    //TODO populate nameList?
    client->sendGenomeList(nameList);
}

//TODO
void ClientActionImpl::alignmentRequest(const std::string& first, const std::string& second)
{
    //TODO generate problem descriptions from the given problem. Add it to problemList.
    scheduler::Problem problem = scheduler::Problem();
    problem.problemID = problemNumber;
    problemNumber++;

    std::vector<char> top_genome;
    std::copy(first.begin(), first.end(), std::back_inserter(top_genome));
    std::vector<char> left_genome;
    std::copy(second.begin(), second.end(), std::back_inserter(left_genome));
    
    std::vector<int> top_numbers;
    std::vector<int> left_numbers;
    for(int i=0; i< top_genome.size(); i++){
        top_numbers.push_back(0);
    }
    for(int i=0; i< left_genome.size(); i++){
        left_numbers.push_back(0);
    }

    problem.top_genome = top_genome;
    problem.left_genome = left_genome;
    problem.top_numbers = top_numbers;
    problem.left_numbers = left_numbers;

    problemList.push_back(problem);
 
    
    //client->sendLocalAlignResponse();
}

