#ifndef __SCHEDULING_H__
#define __SCHEDULING_H__

#include <set>

#include "common/problem.h"

class LeaderClientProtocol;

namespace scheduler {

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
        ProblemDescription * first;

    };

    
    /*class Job
    {
        std::set<Problem*> subproblems;
        std::set<Problem*> initialProblems;
    };*/
    
}

#endif /* __SCHEDULING_H__ */
