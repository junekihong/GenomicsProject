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
    virtual void continueGenomeUpload(unsigned index, const std::vector<unsigned char>& data);
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

void ClientActionImpl::continueGenomeUpload(unsigned index, const std::vector<unsigned char>& data)
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
    std::vector<std::string> nameList;
    nameList.reserve(nameToGenomeLength.size());
    for( std::map<std::string, int>::iterator iter = nameToGenomeLength.begin(); iter != nameToGenomeLength.end(); ++iter )
        nameList.push_back(iter->first);
    client->sendGenomeList(nameList);
}

void ClientActionImpl::alignmentRequest(const std::string& first, const std::string& second)
{
    int firstStartIndex = 0;
    int secondStartIndex = 0;
    
    int firstLength = nameToGenomeLength[first];
    int secondLength = nameToGenomeLength[second];


    // generate problem descriptions from the given problem. Add it to problemList.
    scheduler::Problem problem = scheduler::Problem();
    problem.problemID = problemNumber;
    problemNumber.increment();

    std::vector<unsigned char> top_genome = storage->queryByName(first, firstStartIndex, firstLength);
    std::vector<unsigned char> left_genome = storage->queryByName(second, secondStartIndex, secondLength);
    
#ifdef DEBUG
    std::cout << "top genome = " << std::string(top_genome.begin(), top_genome.end()) << "\n";
#endif

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

