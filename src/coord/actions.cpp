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

// Problem parts that are not ready to be thrown into the problemList
std::map<ProblemID, scheduler::Problem> lockedProblemChunks;

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
    scheduler::Problem currentChunk =  lockedProblemChunks[resp->problemDescription.problemID];
    Solution* currentSolution = resp->sol;
    int** currentMatrix = currentSolution->matrix.matrix;
    

    currentChunk.right->left = true;
    currentChunk.down->up = true;
    currentChunk.right_down->left_up = true;
    
    





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

    int firstStartIndex = 0;
    int secondStartIndex = 0;
    
    int firstLength = nameToGenomeLength[first];
    int secondLength = nameToGenomeLength[second];


    //We allocate a problem chunk matrix to keep track of it all.
    std::vector<std::vector<scheduler::Problem> > problemChunkMatrix;
    for(int i = 0; i < divisionConstant; i++) {
        problemChunkMatrix.push_back(std::vector<scheduler::Problem>());
        for(int j = 0; j < divisionConstant; j++){
            problemChunkMatrix[i].push_back(scheduler::Problem());
        }
    }

    int firstSubLength = firstLength / divisionConstant;
    int secondSubLength = secondLength / divisionConstant;

    for(int j = 0; j < divisionConstant; j++)
    {
        int firstIndex = j * firstSubLength;
        std::string firstChunk = first.substr(firstIndex, firstSubLength);
        
        for(int i = 0; i < divisionConstant; i++)
        {
            int secondIndex = i * secondSubLength;
            std::string secondChunk = second.substr(secondIndex, secondSubLength);         
            problemChunkMatrix[i][j].problemID = problemNumber;
            problemNumber++;

            std::vector<char> top_genome = storage->queryByName(first, firstIndex, firstChunk.length());
            std::vector<char> left_genome = storage->queryByName(second, secondIndex, secondChunk.length());
            problemChunkMatrix[i][j].top_genome = top_genome;
            problemChunkMatrix[i][j].left_genome = left_genome;
            problemChunkMatrix[i][j].requestor = client;
            
            if(i > 0) {
                problemChunkMatrix[i-1][j].down = &problemChunkMatrix[i][j];
            }
            if(j > 0) {
                problemChunkMatrix[i][j-1].right = &problemChunkMatrix[i][j];
            }
            if(i > 0 && j > 0) {
                problemChunkMatrix[i-1][j-1].right_down = &problemChunkMatrix[i][j];
            }
            if(i > 0 || j > 0) {
                problemChunkMatrix[i][j].first = &problemChunkMatrix[0][0];
            }
            
            // Im currently including all chunks in lockedProblemChunks. Including (0, 0)
            lockedProblemChunks[problemChunkMatrix[i][j].problemID] = problemChunkMatrix[i][j];
        }

    // generate problem descriptions from the given problem. Add it to problemList.
    scheduler::Problem problem = scheduler::Problem();
    problem.problemID = problemNumber;
    problemNumber.increment();

    std::vector<char> top_genome = storage->queryByName(first, firstStartIndex, firstLength);
    std::vector<char> left_genome = storage->queryByName(second, secondStartIndex, secondLength);
    
#ifdef DEBUG
    std::cout << "top genome = " << std::string(top_genome.begin(), top_genome.end()) << "\n";
#endif

    std::vector<int> top_numbers;
    std::vector<int> left_numbers;
    for(unsigned int i=0; i< top_genome.size(); i++){
        top_numbers.push_back(0);
    }
    
    // Initialize all the top numbers.
    for(int j = 0; j < divisionConstant; j++)
    {
        problemChunkMatrix[0][j].up = true;
        for(unsigned int k=0; k< problemChunkMatrix[0][j].top_genome.size(); k++){
            problemChunkMatrix[0][j].top_numbers.push_back(0);
        }
    }

    // Initialize all the left numbers.
    for(int i = 0; i < divisionConstant; i++)
    {
        problemChunkMatrix[i][0].left = true;
        for(unsigned int k=0; k< problemChunkMatrix[i][0].left_genome.size(); k++){
            problemChunkMatrix[i][0].left_numbers.push_back(0);
        }
    }

    scheduler::Problem firstProblem = problemChunkMatrix[0][0];
    problemList.insert(std::pair<ProblemID, scheduler::Problem>(firstProblem.problemID,firstProblem));
                       

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

