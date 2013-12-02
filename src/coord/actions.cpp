#include <algorithm>
#include <map>

#include <unistd.h>

#include "protocol.h"
#include "scheduling.h"

std::map<ProblemID, scheduler::Problem> problemList;
typedef std::map<ProblemID, scheduler::Problem>::iterator ProbIter;
std::vector<std::string> nameList;
int problemNumber = 0;

std::vector<ProblemID> solvedList;

std::set<ProblemID, scheduler::Problem> problemsInProgress;

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
    // Reconcile how problemList is a vector of scheduler::Problem, wheras sendProblemList expects a vector of ProblemDescriptions. What I do is populate a temp vector and send that instead.
    
    std::vector<ProblemDescription> tempProblemList;
    for( ProbIter iter = problemList.begin(); iter != problemList.end(); ++iter )
    {
        ProblemDescription problemDescription = ProblemDescription();
        problemDescription.top_numbers = iter->second.top_numbers;
        problemDescription.left_numbers = iter->second.left_numbers;
        problemDescription.top_genome = iter->second.top_genome;
        problemDescription.left_genome = iter->second.left_genome;
        tempProblemList.push_back(problemDescription);
    }

    worker->sendProblemList(tempProblemList);
}

class ProblemComparator
{
    ProblemID desired_id;
public:
    ProblemComparator(ProblemID _id)
    : desired_id(_id)
    { }
    bool operator()(const ProblemDescription& prob) {
        return desired_id == prob.problemID;
    }
};

void WorkerActionImpl::claimProblems(const std::vector<ProblemID>& problems)
{
    for(unsigned int i = 0; i < problems.size(); i++)
    {
        // if problem is in problemList continue, else return false.
        ProbIter iter = problemList.find(problems[i]);
        if( iter == problemList.end() )
        {
            worker->respondToProblemClaim(false);
            return;
        }
    }
 
    // If we make it here, then all the claimed problems appeared in our problem list. Remove them from the problem list and respond with true.
    for( std::vector<ProblemID>::const_iterator iter = problems.begin(); iter != problems.end(); ++iter)
    {
        problemList.erase(*iter);
    }
    worker->respondToProblemClaim(true);
}

void WorkerActionImpl::recieveSolution(const SolutionCertificate& solution)
{
    solvedList.push_back(solution.problemID);
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

void ClientActionImpl::alignmentRequest(const std::string& first, const std::string& second)
{
    // generate problem descriptions from the given problem. Add it to problemList.
    scheduler::Problem problem = scheduler::Problem();
    problem.problemID = problemNumber;
    problemNumber++;

    std::vector<char> top_genome;
    std::copy(first.begin(), first.end(), std::back_inserter(top_genome));
    std::vector<char> left_genome;
    std::copy(second.begin(), second.end(), std::back_inserter(left_genome));
    
    std::vector<int> top_numbers;
    std::vector<int> left_numbers;
    for(unsigned int i=0; i< top_genome.size(); i++){
        top_numbers.push_back(0);
    }
    for(unsigned int i=0; i< left_genome.size(); i++){
        left_numbers.push_back(0);
    }

    problem.top_genome = top_genome;
    problem.left_genome = left_genome;
    problem.top_numbers = top_numbers;
    problem.left_numbers = left_numbers;
    problem.requestor = client;

    problemList.insert(std::pair<ProblemID, scheduler::Problem>(problem.problemID, problem));

    // The response will get sent back to the client when the solution
    // is given to us by a worker.
}

