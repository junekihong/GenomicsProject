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
}
