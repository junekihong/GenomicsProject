module mdp.worker.worker;

private import std.algorithm : max, min;
private import std.random : uniform;
private import std.stdio : writeln;

private import mdp.common.problem;
import mdp.common.protocol;
private import mdp.common.location;
private import mdp.common.matrix;
private import mdp.common.solution;
import mdp.worker.protocol;

void runWorker(WorkerLeaderProtocol leader, StorageProtocol storage)
{
    import std.random;
    while(true)
    {
        // Get the list of available problems
        ProblemDescription [] problemList = leader.requestProblemList();

        // Select a problem
        // Attempt to claim up to 4 problems if they are available
        // Do this by choosing a starting index and claiming the next 4 problems
        // This isn't really what we want, since the probability of
        // choosing the ends is lower than the middle.
        // On the other hand, we should be using information like the data
        // that we've cached (we don't cache anything yet really) to choose
        // problems that we'd be really good at.  So the bad-ish solution stays

        uint claimCount = min(4, problemList.length);
        ulong startPositions = max(1, problemList.length - claimCount);
        ulong startIndex = uniform(0, startPositions);

        ProblemDescription[] descs = new ProblemDescription[claimCount];
        ProblemID[] problems = new ProblemID[claimCount];
        foreach( uint i; 0 .. claimCount )
        {
            descs[i] = problemList[i + startIndex];
            problems[i] = descs[i].problemID;
        }

        // Try to claim the problems
        bool claimed = leader.claimProblems(ClaimProblems(problems));
        if( !claimed )
            continue; // TODO put this entire process into its own do-while or something
        writeln("Claimed ", problems.length, " problems");

        // Query storage to see if this problem has already been solved
        foreach( ProblemDescription problemDescription; descs )
        {
            // Only query for exact matches
            // TODO eliminate copying the problemDescription,
            // though it *might* be a shallow copy
            QueryByInitialConditions msg = { problemDescription, false};
            QueryResponse queryResponse = storage.queryByInitialConditions(msg);

            SolutionCertificate solutionCertificate;
            solutionCertificate.problemID = problemDescription.problemID;

            if( queryResponse.success && queryResponse.exactMatch )
            {
                debug(1) {
                    writeln("Cache hit!");
                }
                solutionCertificate.solutionID = queryResponse.sol.id;
            }
            else
            {
                Solution sol = solveProblem(problemDescription);

                // TODO eliminate copies, see queryByInitialConditions above
                InsertSolution sol_msg = {problemDescription, sol};
                storage.insertSolution(sol_msg);
                solutionCertificate.solutionID = problemDescription.problemID;
            }

            leader.sendSolution(SendSolutionReport(solutionCertificate));
        }
    }
}

Solution solveProblem(in ProblemDescription problemDescription)
{
    Solution solution;
    solution.id = problemDescription.problemID;

    // DEBUG writeln("Solving problem ", solution.id.idnum)
    solution.matrix = Matrix(problemDescription.top_numbers, problemDescription.left_numbers);

    writeln("Solving the problem!");
    LocationValuePair maxValuePair = solution.matrix.localAlignment(problemDescription.top_genome, problemDescription.left_genome);

    solution.maxValue = maxValuePair.value; // TODO kill this pair assignment
    solution.maxValueLocation =  maxValuePair.location;

    return solution;
}
