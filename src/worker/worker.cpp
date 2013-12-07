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
            usleep(1000);
            continue;
        }
        std::cout << "list is " << problemList.size() << " long\n";
        
        // Select a problem
        /*ProblemDescription problemDescription = problemList.at(0);
        ProblemID problemID = problemDescription.problemID;
        problems.push_back(problemID);*/
        std::vector<ProblemDescription> descs;
        int claimCount = std::min<int>(4, problemList.size());
        int start_index = std::max<int>(1, problemList.size() - claimCount);
        if( start_index > 0 ) {
            start_index = rand() % start_index;
        }
        for( int i = 0; i < claimCount; ++i ) {
            descs.push_back(problemList.at(i + start_index));
            problems.push_back(problemList.at(i + start_index).problemID);
        }
        
        // Claim a problem
        bool claimed = leader.claimProblems(problems);
        if(!claimed){
            continue;
        }
        std::cout << "Claimed " << problems.size() << " problems\n";

        // Query storage to check for a cache hit.
        bool wantPartials = false;
        for( int i = 0; i < claimCount; ++i ) {
            ProblemDescription& problemDescription = descs[i];
            QueryResponse* queryResponse = storage.queryByInitialConditions(problemDescription, wantPartials);

            SolutionCertificate solutionCertificate;
            if(queryResponse->success && queryResponse->exactMatch)
            {
//#ifdef DEBUG
                std::cout << "Cache hit!\n";
//#endif
                //solutionCertificate = queryResponse->solutionCertificate;
                solutionCertificate.problemID = problemDescription.problemID;
                Solution* solution = queryResponse->sol;
                solutionCertificate.solutionID = solution->id;
            }
            else
            {
                // Solve the problem manually.
                Solution solution = solveProblem(problemDescription);
                while(!storage.insertSolution(problemDescription, solution)){
                    //usleep(1000);
                }
                solutionCertificate.problemID = problemDescription.problemID;
                solutionCertificate.solutionID = problemDescription.problemID;
            }

            delete queryResponse;

            // Send solution back.
            leader.sendSolution(solutionCertificate);
         
        }
    }
}



// Function called by the worker to solve a problem given its description and return the solution.
Solution solveProblem(ProblemDescription problemDescription)
{
    Solution solution;
    solution.id = problemDescription.problemID;
#ifdef DEBUG
    std::cout << "Solving problem " << solution.id.idnum << "\n";
#endif
    solution.matrix = Matrix(problemDescription.top_numbers, problemDescription.left_numbers);

    LocationValuePair maxValuePair = solution.matrix.localAlignment(problemDescription.top_genome, problemDescription.left_genome);
    
    solution.maxValue = maxValuePair.value;
    solution.maxValueLocation = maxValuePair.location;

    return solution;
}
