#ifndef __SCHEDULING_H__
#define __SCHEDULING_H__

#include <set>

#include "common/problem.h"

class LeaderClientProtocol;

namespace scheduler {

    class Problem : public ProblemDescription
    {
        public:
        // The neighboring problems, NULL if no neighboring problem
        ProblemDescription * right;
        ProblemDescription * down;
        ProblemDescription * right_down;
        LeaderClientProtocol * requestor;
    };
    
    /*class Job
    {
        std::set<Problem*> subproblems;
        std::set<Problem*> initialProblems;
    };*/
    
}

extern ProblemID problemNumber;
extern std::map<std::string, int> genomes;

#endif /* __SCHEDULING_H__ */
