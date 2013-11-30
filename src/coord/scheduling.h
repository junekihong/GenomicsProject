#ifndef __SCHEDULING_H__
#define __SCHEDULING_H__

#include <set>

#include "common/problem.h"

namespace scheduler {

    class Problem : public ProblemDescription
    {
        public:
        // The neighboring problems, NULL if no neighboring problem
        ProblemDescription * right;
        ProblemDescription * down;
        ProblemDescription * right_down;
    };
    
    class Job
    {
        std::set<Problem*> subproblems;
        std::set<Problem*> initialProblems;
    };
    
}

#endif /* __SCHEDULING_H__ */
