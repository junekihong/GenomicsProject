#include <fstream>
#include <iostream>
#include <map>
#include <set>

#include <boost/filesystem.hpp>

#include "common/problem.h"
#include "common/protocol.h"
#include "common/solution.h"
#include "common/util.h"

#include "solutionOps.h"

class CompleteSolution
{
public:
    ProblemDescription desc;
    // TODO store solution on disk
    Solution sol;
    
    CompleteSolution()
    : desc(), sol()
    { }
    
    CompleteSolution(const ProblemDescription& d, const Solution& s)
    : desc(d), sol(s)
    { }
};

static bool compare_top_numbers(const CompleteSolution* left, const CompleteSolution * right)
{
    if( left->desc.corner != right->desc.corner )
        return left->desc.corner < right->desc.corner;
    return left->desc.top_numbers < right->desc.top_numbers;
}
static bool compare_left_numbers(const CompleteSolution* left, const CompleteSolution * right)
{
    if( left->desc.corner != right->desc.corner )
        return left->desc.corner < right->desc.corner;
    return left->desc.left_numbers < right->desc.left_numbers;
}
static bool compare_top_genomes(const CompleteSolution* left, const CompleteSolution * right)
{
    return left->desc.top_genome < right->desc.top_genome;
}
static bool compare_left_genomes(const CompleteSolution* left, const CompleteSolution * right)
{
    return left->desc.left_genome < right->desc.left_genome;
}

static const boost::filesystem::path solutionRoot("solutions");

// This map owns the solutions
std::map<ProblemID, const CompleteSolution*> solutionById;
typedef std::pair<ProblemID, const CompleteSolution*> solution_pair;

// other indexes of solutions...
std::set<const CompleteSolution*, bool(*)(const CompleteSolution*, const CompleteSolution*)> topNumberIndex(compare_top_numbers);
std::set<const CompleteSolution*, bool(*)(const CompleteSolution*, const CompleteSolution*)> leftNumberIndex(compare_left_numbers);
std::set<const CompleteSolution*, bool(*)(const CompleteSolution*, const CompleteSolution*)> topGenomeIndex(compare_top_genomes);
std::set<const CompleteSolution*, bool(*)(const CompleteSolution*, const CompleteSolution*)> leftGenomeIndex(compare_left_genomes);

void initializeSolutionSystem()
{
    if( !boost::filesystem::exists(solutionRoot) )
    {
        boost::filesystem::create_directory(solutionRoot);
        return;
    }
    
    if( !boost::filesystem::is_directory(solutionRoot) )
    {
        throw std::runtime_error("file \"" + solutionRoot.generic_string<std::string>() + "\" exists, but cannot"
                                 " be used as the genome root because it is not a directory");
    }
    
    // solution root exists, so find the solutions in it
    boost::filesystem::directory_iterator dir_end = boost::filesystem::directory_iterator();
    for( boost::filesystem::directory_iterator iter = boost::filesystem::directory_iterator(solutionRoot);
        iter != dir_end;
        ++iter )
    {
        std::string name = iter->path().filename().generic_string<std::string>();
        std::ifstream input(iter->path().generic_string<std::string>().c_str());
        
        CompleteSolution * sol = new CompleteSolution;
        readProblemDescription(input, sol->desc);
        readSolution(input, sol->sol, "Error reading solution");
        
        solutionById[sol->sol.id] = sol;
        // TODO add to other indices
    }
}

void insertSolution(const ProblemDescription& prob, const Solution& s)
{
    CompleteSolution * sol = new CompleteSolution(prob, s);
    solutionById[sol->sol.id] = sol;
    // TODO add to other indices
    
    std::ofstream strm((solutionRoot / toString(sol->sol.id.idnum)).generic_string<std::string>().c_str());
    sendProblemDescription(strm, prob, "Error writing problem description " + toString(prob.problemID.idnum));
    sendSolution(strm, s, "Error writing solution " + toString(sol->sol.id.idnum));
}

void queryByID(ProblemID id, bool solution_wanted, QueryResponse& resp)
{
    if( !solutionById.count(id) )
    {
        resp.success = false;
        return;
    }
    
    const CompleteSolution* sol = solutionById[id];
    resp.success = true;
    resp.exactMatch = true;
    resp.problemDescription = sol->desc;
    resp.maxValue = sol->sol.maxValue;
    resp.location = sol->sol.maxValueLocation;
    
    if( solution_wanted )
    {
        resp.sol = new Solution;
        (*resp.sol) = sol->sol;
    }
    else {
        resp.sol = NULL;
    }
}

void queryByConditions(const ProblemDescription& desc, bool partialsWanted, QueryResponse& resp)
{
    
}
