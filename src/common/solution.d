module mdp.common.solution;

import mdp.common.matrix;
import mdp.common.location;
import mdp.common.problem;

struct SolutionCertificate
{
    ProblemID problemID;
    ProblemID solutionID;
}

struct Solution
{
    ProblemID id;

    int maxValue;
    Location maxValueLocation;
    Matrix matrix;

    @property Solution dup() inout
    {
        Solution result;
        result.id = id;
        result.maxValue = maxValue;
        result.maxValueLocation = maxValueLocation;
        result.matrix = matrix; // Matrix assignment does a deep copy
        return result;
    }

    @property immutable(Solution) idup()
    {
        return immutable Solution(
            id, maxValue, maxValueLocation, matrix.idup);
    }

    void opAssign(in Solution other) shared
    {
        id = other.id;
        maxValue = other.maxValue;
        maxValueLocation = other.maxValueLocation;
        matrix = other.matrix;
    }
}
