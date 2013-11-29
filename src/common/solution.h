#ifndef __SOLUTION_H__
#define __SOLUTION_H__

#include <vector>
#include "common/location.h"
#include "common/matrix.h"
#include "common/problem.h"

class SolutionCertificate
{
    public:
	ProblemID problemID;
    ProblemID solutionID;
};

class Solution
{
    public:
	ProblemID problemID;
    ProblemID solutionID;
	
	int maxValue;
	Location maxValueLocation;
//	Matrix matrix;
};



#endif // __SOLUTION_H__
