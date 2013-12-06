#ifndef __SOLUTION_H__
#define __SOLUTION_H__

#include <vector>

#include <msgpack.hpp>

#include "common/matrix.h"
#include "common/location.h"
#include "common/problem.h"

class SolutionCertificate
{
    public:
	ProblemID problemID;
    ProblemID solutionID;
    
    MSGPACK_DEFINE(problemID, solutionID);
};

class Solution
{
    public:
    ProblemID id;
	
	int maxValue;
	Location maxValueLocation;
	Matrix matrix;
    
    MSGPACK_DEFINE(id, maxValue, maxValueLocation, matrix);
};

#endif // __SOLUTION_H__
