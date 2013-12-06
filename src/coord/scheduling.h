#ifndef __SCHEDULING_H__
#define __SCHEDULING_H__

#include <map>
#include <string>

#include "common/problem.h"
#include "common/solution.h"

class LeaderClientProtocol;

namespace scheduler {
    class Job;

    class Problem : public ProblemDescription
    {
        public:
        Problem();
        
        // The neighboring problems, NULL if no neighboring problem. Forward pointers.
        Problem * right;
        Problem * down;
        Problem * right_down;
        LeaderClientProtocol * requestor;

        // Backwards booleans
        bool left;
        bool up;
        bool left_up;

        // The first problem. We use this to start at the beginning and put together the entire matrix.
        Problem * first;

        // Pointer back up to the job that is keeping track of it.
        Job* job;

    };

    class Job
    {
    public:
        std::vector<std::vector<Problem> > subproblemMatrix;
        std::vector<Problem> availableProblems;
        std::map<ProblemID, Problem> problemMap;

        int divisionConstant;
        LeaderClientProtocol* client;
    
        bool finished;
    
    public:
        Job();
        Job(std::vector<char> first, std::vector<char> second, LeaderClientProtocol* requestor, ProblemID problemNumber, int divisionConstant);
        
        std::vector<Problem> getAvailableProblems();

        void update(ProblemID problemID, Solution solution);
        
        Solution combineChunks();

    };
    
}

extern ProblemID problemNumber;
extern std::map<std::string, int> nameToGenomeLength;

#endif /* __SCHEDULING_H__ */
