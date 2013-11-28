#include "worker.h"

#include <unistd.h>
#include "common/solution.h"

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
      ProblemDescription problem = problemList.at(0);
      ProblemID ID = problem.id;
      problems.push_back(ID);
      
      // Claim a problem
      bool claimed = leader.claimProblems(problems);
      if(!claimed){
	continue;
      }

      //TODO take problem. produce solution.
      SolutionCertificate solution;

      // Send solution back.
      leader.sendSolution(solution);


    }
}


