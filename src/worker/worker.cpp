#include "worker.h"

#include <unistd.h>
#include "common/solution.h"
#include "common/protocol.h"
#include "common/matrix.h"
#include "common/pair_location_value.h"

void runWorker(WorkerLeaderProtocol& leader, StorageProtocol& storage)
{
    while(true)
    {
        std::vector<ProblemDescription> problemList;
        std::vector<ProblemID> problems;
        
        // Get a problem list.
        leader.requestProblemList(problemList);
        
        if(problemList.empty()){
            sleep(1);
            continue;
        }
        
        // Select a problem
        ProblemDescription problemDescription = problemList.at(0);
        ProblemID problemID = problemDescription.problemID;
        problems.push_back(problemID);
        
        // Claim a problem
        bool claimed = leader.claimProblems(problems);
        if(!claimed){
            continue;
        }

        // Query storage to check for a cache hit.
        bool wantPartials = false;
        QueryResponse* queryResponse = storage.queryByInitialConditions(problemDescription, wantPartials);

        SolutionCertificate solutionCertificate;
        if(queryResponse->success && queryResponse->exactMatch)
        {
            //solutionCertificate = queryResponse->solutionCertificate;
            solutionCertificate.problemID = problemDescription.problemID;
            Solution* solution = queryResponse->sol;
            solutionCertificate.solutionID = solution->id;
        }
        else
        {
            // Solve the problem manually.
            Solution solution = solveProblem(problemDescription);
            while(!storage.insertSolution(solution)){
                sleep(1);
            }
            solutionCertificate.problemID = problemDescription.problemID;
            solutionCertificate.solutionID = problemDescription.problemID;
        }

        delete queryResponse;

        // Send solution back.
        leader.sendSolution(solutionCertificate);        
    }
}



// Function called by the worker to solve a problem given its description and return the solution.
Solution solveProblem(ProblemDescription problemDescription)
{
    Solution solution;
    solution.id = problemDescription.problemID;
    solution.matrix = Matrix(problemDescription.top_numbers, problemDescription.left_numbers);

    LocationValuePair maxValuePair = solution.matrix.localAlignment(problemDescription.top_genome, problemDescription.left_genome);
    
    solution.maxValue = maxValuePair.value;
    solution.maxValueLocation = maxValuePair.location;

    return solution;
}
