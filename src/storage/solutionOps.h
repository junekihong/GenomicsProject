#ifndef __SOLUTION_OPS_H__
#define __SOLUTIOn_OPS_H__

#include <string>
#include <vector>

#include "common/problem.h"

class Solution;

void initializeSolutionSystem();
void insertSolution(const ProblemDescription& prob, const Solution& sol);

void queryByID(ProblemID id, bool solution_wanted, QueryResponse& resp);
void queryByConditions(const ProblemDescription& desc, bool partialsWanted, QueryResponse& resp);

#endif /* __GENOME_OPS_H__ */
