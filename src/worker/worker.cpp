#include "worker.h"

#include <unistd.h>
#include "common/solution.h"
#include "common/query_response.h"

void runWorker(WorkerLeaderProtocol& leader, StorageProtocol& storage){
    std::vector<ProblemDescription> problemList;
    std::vector<ProblemID> problems;
    
    while(true)
    {
        problemList.clear();
        problems.clear();
        
        // Get a problem list.
        leader.requestProblemList(problemList);
        
        if(problemList.empty()){
            sleep(1);
            continue;
        }
        
        // Select a problem
        ProblemDescription problemDescription = problemList.at(0);
        ProblemID problemID = problemDescription.id;
        problems.push_back(problemID);
        
        // Claim a problem
        bool claimed = leader.claimProblems(problems);
        if(!claimed){
            continue;
        }

        // Query storage to check for a cache hit.
        int queryRequestID = 0;
        bool queryFlag = false;
        QueryResponse queryResponse = storage.queryByInitialConditions(queryRequestID, problemDescription, queryFlag);
        

        SolutionCertificate solutionCertificate;
        if(queryResponse.exactMatch)
        {
            solutionCertificate = queryResponse.solutionCertificate;
        }
        else if(queryResponse.potentialMatch)
        {
            // TODO
        }
        else
        {
            // Solve the problem manually.
            Solution solution = solveProblem(problemDescription);
            while(!storage.insertSolution(solution)){
                sleep(1);
            }
            queryResponse = storage.queryByProblemID(problemID);
            solutionCertificate = queryResponse.solutionCertificate;
        }

        // Send solution back.
        leader.sendSolution(solutionCertificate);        
    }
}




Solution solveProblem(ProblemDescription problemDescription){
    Solution solution;
    return solution;
}
