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
        ProblemID ID = problemDescription.id;
        problems.push_back(ID);
        
        // Claim a problem
        bool claimed = leader.claimProblems(problems);
        if(!claimed){
            continue;
        }
        

        
        int queryRequestID = 0;
        bool queryFlag = false;
        QueryResponse queryResponse = storage.queryByInitialConditions(queryRequestID, problemDescription, queryFlag);
        
        //TODO take problem. produce solution.
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
            // TODO solve the problem manually.
        }

        // Send solution back.
        leader.sendSolution(solutionCertificate);
        
        
    }
}


