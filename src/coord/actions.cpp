#include <algorithm>
#include <map>

#include <unistd.h>

#include "protocol.h"
#include "scheduling.h"

// Iterator typedefs
typedef std::map<ProblemID, scheduler::Problem>::iterator ProbMapIter;
typedef std::vector<ProblemID>::const_iterator ProbIter;

// The list of problems.
std::map<ProblemID, scheduler::Problem> problemList;

// The list of jobs.
std::vector<scheduler::Job> jobs;

ProblemID problemNumber(0);

// Names of genomes.
std::vector<std::string> nameList;
std::map<std::string, int> nameToGenomeLength;

std::map<ProblemID, scheduler::Problem> problemsInProgress;
std::map<ProblemID, SolutionCertificate> solvedProblems;

// List of workers that are ready for a new problem.
std::vector<LeaderWorkerProtocol *> availableWorkers;

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
    // If our problem list is empty, we keep our worker waiting around until we can send them a populated list.
    if(problemList.empty())
    {
        availableWorkers.push_back(worker);
        return;
    }

    // Reconcile how problemList is a vector of scheduler::Problem, wheras sendProblemList expects a vector of ProblemDescriptions. What I do is populate a temp vector and send that instead.
    std::vector<ProblemDescription> tempProblemList;
    for( ProbMapIter iter = problemList.begin(); iter != problemList.end(); ++iter )
    {
        ProblemDescription problemDescription = iter->second;
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
        ProbMapIter iter = problemList.find(problems[i]);
        if( iter == problemList.end() )
        {
            worker->respondToProblemClaim(false);
            return;
        }
    }
 
    // If we make it here, then all the claimed problems appeared in our problem list. Remove them from the problem list and respond with true.
    for( ProbIter iter = problems.begin(); iter != problems.end(); ++iter)
    {
        // FIXME, doing the lookup into problemList again...
        problemsInProgress.insert(std::make_pair(*iter, problemList.at(*iter)));
        problemList.erase(*iter);
    }
    worker->respondToProblemClaim(true);
}

void WorkerActionImpl::recieveSolution(const SolutionCertificate& solution)
{
    solvedProblems.insert(std::make_pair(solution.problemID, solution));
    scheduler::Problem problem = problemsInProgress[solution.problemID];
    problemsInProgress.erase(solution.problemID);
    
    QueryResponse * resp = storage->queryByProblemID(solution.solutionID, true);
    if( resp->success == false || resp->exactMatch == false || resp->sol == NULL )
        throw std::runtime_error("Error getting the solution from storage");

    
    //TODO need to check the solution against our lockedProblemChunks
    ProblemID problemID = resp->problemDescription.problemID;
    scheduler::Job job;
    bool foundJob = false;
    for(unsigned int i = 0; i < jobs.size(); i++)
    {
        job = jobs[i];
        if(job.problemMap.find(problemID) != job.problemMap.end())
        {
            foundJob = true;
            break;
        }
    }

    if(foundJob)
    {
        job.update(problemID, *(resp->sol));
        std::vector<scheduler::Problem> newProblems = job.getAvailableProblems();
        for(unsigned int i = 0; i < newProblems.size(); i++)
        {
            scheduler::Problem problem = newProblems[i];
            problemList[problem.problemID] = problem;
        }
        
        if(job.finished)
        {
            Solution solution = job.combineChunks();
            
            job.client->sendLocalAlignResponse(solution);
            destroy_socket(job.client->getSocket());
        }
        
    }
    else 
    {
        problem.requestor->sendLocalAlignResponse(*(resp->sol));
        destroy_socket(problem.requestor->getSocket());
    } 
    delete resp;

    
    problem.requestor->sendLocalAlignResponse(*(resp->sol));
    delete resp;

    destroy_socket(problem.requestor->getSocket());
}


class ClientActionImpl : public ClientActions
{
    LeaderClientProtocol * client;
    std::string storedName;

    public:
    ClientActionImpl(LeaderClientProtocol * c);
    
    virtual void startGenomeUpload(const std::string& name, unsigned length);
    virtual void continueGenomeUpload(unsigned index, const std::vector<char>& data);
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
    storedName = name;
    storage->createNewGenome(name, length);
    nameToGenomeLength.insert(std::make_pair(name, length));
}

void ClientActionImpl::continueGenomeUpload(unsigned index, const std::vector<char>& data)
{   
    storage->insertGenomeData(storedName, index, data);
}

void ClientActionImpl::finishGenomeUpload()
{
    client->sendGenomeUploadResponse();
}


void ClientActionImpl::listGenomes()
{
    // TODO without the insane amount of copying
    std::vector<std::string> nameList(nameToGenomeLength.size());
    for( std::map<std::string, int>::iterator iter = nameToGenomeLength.begin(); iter != nameToGenomeLength.end(); ++iter )
        nameList.push_back(iter->first);
    client->sendGenomeList(nameList);
}

void ClientActionImpl::alignmentRequest(const std::string& first, const std::string& second)
{

    int divisionConstant = 2;
    int firstLength = nameToGenomeLength.find(first)->second;
    int secondLength = nameToGenomeLength.find(second)->second;
    if(firstLength <= 1 || secondLength <= 1) {
        divisionConstant = 1;
    }

    std::vector<char> genome1 = storage->queryByName(first, 0, firstLength);
    std::vector<char> genome2 = storage->queryByName(second, 0, secondLength);

    scheduler::Job job = scheduler::Job(genome1, genome2, client, problemNumber, divisionConstant);
    jobs.push_back(job);

    std::vector<scheduler::Problem> availableProblems = job.getAvailableProblems();
    for(unsigned int i = 0; i < availableProblems.size(); i++){
        scheduler::Problem problem = availableProblems[i];
        problemList.insert(std::pair<ProblemID, scheduler::Problem>(problem.problemID, problem));
    }

    // Make a list of problems on the spot
    std::vector<ProblemDescription> tempProblemList;
    for( ProbMapIter iter = problemList.begin(); iter != problemList.end(); ++iter )
    {
        ProblemDescription problemDescription = iter->second;
        tempProblemList.push_back(problemDescription);
    }
    // We actively give the problem list out to our available workers
    for(unsigned int i = 0; i < availableWorkers.size(); i++)
    {
        availableWorkers[i]->sendProblemList(tempProblemList);
    }
    availableWorkers.clear();
}

