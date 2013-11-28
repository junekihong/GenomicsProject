#ifndef __QUERY_RESPONSE_H__
#define __QUERY_RESPONSE_H__

#include "common/problem.h"
#include "common/solution.h"

class QueryResponse
{
public:
    int queryResponseID;
    bool potentialMatch;
    bool exactMatch;
    
    // Given problem description.
    ProblemDescription problemDescription;

    // Solution description. Maximum value and location in the matrix.
    int maxValue;
    Location location;

    // The solution.
    SolutionCertificate solutionCertificate;
};


#endif //__QUERY_RESPONSE_H__
