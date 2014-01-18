module mdp.storage.solutionOps;

import std.array;
import std.conv : to;
import std.stdio;

import msgpack;

import vibe.core.file;

import mdp.common.problem;
import mdp.common.protocol;
import mdp.common.solution;

// Initialized to 0 by defualt
public ProblemID nextSolutionID;

private {
    immutable solutionPathEntry = PathEntry("solutions");
    immutable solutionRoot = Path(solutionPathEntry);
    CompleteSolution[ProblemID] solutionById;
	alias solutionArray = CompleteSolution*[];
	solutionArray[int[]] topNumbersIndex;
	solutionArray[int[]] leftNumbersIndex;
	solutionArray[ubyte[]] topGenomeIndex;
	solutionArray[ubyte[]] leftGenomeIndex;
    //RedBlackTree!(CompleteSolution ref)
}


struct CompleteSolution
{
    // TODO put solutions on disk, cache / organize these well
    ProblemDescription desc;
    Solution sol;
}

private bool compare_numbers(string side)(in CompleteSolution left, in CompleteSolution right)
{
    if( left.desc.corner != right.desc.corner )
        return left.desc.corner < right.desc.corner;
    return mixin("left.desc. "~ side ~ "_numbers") < mixin("right.desc." ~ side ~ "_numbers");
}

void initializeSolutionSystem()
{
    if( !existsFile(solutionRoot) )
    {
        writeln("Creating new solutions directory");
        createDirectory(solutionRoot);
        return;
    }

    if( !getFileInfo(solutionRoot).isDirectory )
    {
        throw new Exception("File \"" ~ to!string(solutionRoot) ~ "\" exists but cannot"
                " be used as the solution root because it is not a directory.");
    }

    // solution root exists, so read in the solutions
    // this caused crashes in the C++ version, so it's commented out for now
    //listDirectory(solutionRoot, &addSolutionFromFile );

    debug(1) {
        //writeln("There are ", solutionById.length, " solutions");
    }
}

void insertIntoIndex(string side, string kind)(ref CompleteSolution sol)
{
    import std.uni : toLower, toUpper;
    enum key_name = "sol.desc." ~ side ~ "_" ~ toLower(kind[0..1]) ~ kind[1..$];
    enum index_name = side ~ toUpper(kind[0..1]) ~ kind[1..$] ~ "Index";
	if( mixin(key_name) !in mixin(index_name) )
		mixin(index_name)[mixin(key_name).idup] = [&sol];
    else
	    mixin(index_name)[mixin(key_name).idup] ~= &sol;
}

// TODO make these const / in etc.
void insertSolution(in InsertSolution msg)
{
    auto sol = CompleteSolution(msg.prob.dup, msg.solution.dup);
    solutionById[msg.solution.id] = sol;

    // TODO insert into other indicies
    insertIntoIndex!("top", "numbers")(sol);
    insertIntoIndex!("left", "numbers")(sol);
    insertIntoIndex!("top", "genome")(sol);
    insertIntoIndex!("left", "genome")(sol);

    FileStream output = openFile(solutionRoot ~ to!string(sol.sol.id), FileMode.createTrunc);
    auto pack = packer(appender!(ubyte[])());
    pack.pack(sol);
    output.write(pack.stream().data);
    output.close();
}

shared(QueryResponse) queryByID(in QueryByProblemID msg)
{
    auto resp = new shared QueryResponse();
    if( msg.prob !in solutionById ) {
        resp.success = false;
        return resp;
    }

    CompleteSolution sol = solutionById[msg.prob];
    resp.success = true;
    resp.exactMatch = true;
    resp.problemDescription = sol.desc.dup;
    resp.maxValue = sol.sol.maxValue;
    resp.location =  sol.sol.maxValueLocation;

    if( msg.entireSolution )
    {
        resp.sol = new shared Solution();
        (*resp.sol) = sol.sol.dup;
    }
    else {
        resp.sol = null;
    }

    return resp;
}

shared(QueryResponse) queryByConditions(in QueryByInitialConditions msg)
{
    if( !msg.wantPartials ) {
        return queryExactly(msg.problemDescription);
    }
    else {
        auto resp = new shared QueryResponse();
        resp.success = false;
        return resp;
    }
}

private shared(QueryResponse) queryExactly(in ProblemDescription desc)
{
    import std.algorithm : canFind, filter;

    auto resp = new shared QueryResponse();
    CompleteSolution*[] firstMatches = topGenomeIndex[desc.top_genome];
    CompleteSolution*[] nextMatches = leftGenomeIndex[desc.left_genome];
    auto firstFilter = filter!( m => canFind(firstMatches, m) )(nextMatches);
    if( firstFilter.empty ) {
        resp.success = false;
        return resp;
    }

    nextMatches = topNumbersIndex[desc.top_numbers];
    auto secondFilter = filter!( m => canFind(firstFilter, m) )(nextMatches);
    if( secondFilter.empty ) {
        resp.success = false;
        return resp;
    }

    nextMatches = leftNumbersIndex[desc.left_numbers];
    auto thirdFilter = filter!( m => canFind(secondFilter, m) )(nextMatches);
    if( thirdFilter.empty ) {
        resp.success = false;
        return resp;
    }

    CompleteSolution * answer = thirdFilter.front();
    resp.success = true;
    resp.exactMatch = true;
    resp.problemDescription = (*answer).desc;
    resp.maxValue = (*answer).sol.maxValue;
    resp.location = (*answer).sol.maxValueLocation;
    resp.sol = new shared Solution();
    (*resp.sol) = (*answer).sol;

    return resp;
}
