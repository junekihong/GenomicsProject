#include <algorithm>
#include <fstream>
#include <iostream>
#include <iterator>
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
    
    MSGPACK_DEFINE(desc, sol);
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
ProblemID nextSolutionID(0);

// other indexes of solutions...
std::set<const CompleteSolution*, bool(*)(const CompleteSolution*, const CompleteSolution*)> topNumberIndex(compare_top_numbers);
std::set<const CompleteSolution*, bool(*)(const CompleteSolution*, const CompleteSolution*)> leftNumberIndex(compare_left_numbers);
std::set<const CompleteSolution*, bool(*)(const CompleteSolution*, const CompleteSolution*)> topGenomeIndex(compare_top_genomes);
std::set<const CompleteSolution*, bool(*)(const CompleteSolution*, const CompleteSolution*)> leftGenomeIndex(compare_left_genomes);

typedef std::set<const CompleteSolution*, bool(*)(const CompleteSolution*, const CompleteSolution*)>::iterator IndexIter;

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
    /*boost::filesystem::directory_iterator dir_end = boost::filesystem::directory_iterator();
    for( boost::filesystem::directory_iterator iter = boost::filesystem::directory_iterator(solutionRoot);
        iter != dir_end;
        ++iter )
    {
        std::string name = iter->path().filename().generic_string<std::string>();
        // Ignore hidden files
        if( !name.size() || name[0] == '.' )
            continue;
        std::ifstream input(iter->path().generic_string<std::string>().c_str());
        boost::uintmax_t length = file_size(iter->path());
        msgpack::unpacker unpack(length);
        input.read(unpack.buffer(), length);
        input.close();
        unpack.buffer_consumed(length);
        
        CompleteSolution * sol = new CompleteSolution;
        read(unpack, *sol);
        
        solutionById[sol->sol.id] = sol;
        topNumberIndex.insert(sol);
        leftNumberIndex.insert(sol);
        topGenomeIndex.insert(sol);
        leftGenomeIndex.insert(sol);
        
        if( sol->sol.id.idnum >= nextSolutionID.idnum )
        {
            nextSolutionID.idnum = sol->sol.id.idnum + 1;
        }
    }*/
}

void insertSolution(const ProblemDescription& prob, const Solution& s)
{
    CompleteSolution * sol = new CompleteSolution(prob, s);
    solutionById[sol->sol.id] = sol;
    topNumberIndex.insert(sol);
    leftNumberIndex.insert(sol);
    topGenomeIndex.insert(sol);
    leftGenomeIndex.insert(sol);
    
    msgpack::sbuffer sbuf;
    msgpack::pack(&sbuf, *sol);
    std::ofstream strm((solutionRoot / toString(sol->sol.id.idnum)).generic_string<std::string>().c_str());
    strm.write(sbuf.data(), sbuf.size());
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

static void queryExactly(const ProblemDescription& desc, QueryResponse& resp);
static void queryApproximately(const ProblemDescription& desc, QueryResponse& resp);

void queryByConditions(const ProblemDescription& desc, bool partialsWanted, QueryResponse& resp)
{
    if( !partialsWanted ) {
        queryExactly(desc, resp);
        return;
    }
    else {
        queryApproximately(desc, resp);
    }
}

static void queryExactly(const ProblemDescription& desc, QueryResponse& resp)
{
    CompleteSolution dummySolution;
    dummySolution.desc = desc;
    std::set<const CompleteSolution*> matches[2];
    
    std::pair<IndexIter, IndexIter> top_num_range = topNumberIndex.equal_range(&dummySolution);
    if( top_num_range.first == top_num_range.second )
    {
        resp.success = false;
        return;
    }
    std::pair<IndexIter, IndexIter> left_num_range = leftNumberIndex.equal_range(&dummySolution);
    if( left_num_range.first == left_num_range.second )
    {
        resp.success = false;
        return;
    }
    std::set_intersection(top_num_range.first, top_num_range.second, left_num_range.first, left_num_range.second, std::inserter(matches[0], matches[0].begin()));
    if( matches[0].size() == 0 )
    {
        resp.success = false;
        return;
    }
    std::pair<IndexIter, IndexIter> top_gen_range = topGenomeIndex.equal_range(&dummySolution);
    if( top_gen_range.first == top_gen_range.second )
    {
        resp.success = false;
        return;
    }
    std::pair<IndexIter, IndexIter> left_gen_range = leftGenomeIndex.equal_range(&dummySolution);
    if( left_gen_range.first == left_gen_range.second )
    {
        resp.success = false;
        return;
    }
    
    std::set_intersection(top_gen_range.first, top_gen_range.second, left_gen_range.first, left_gen_range.second, std::inserter(matches[1], matches[1].begin()));
    if( matches[1].size() == 0 )
    {
        resp.success = false;
        return;
    }
    
    std::set<const CompleteSolution*> result;
    std::set_intersection(matches[0].begin(), matches[0].end(), matches[1].begin(), matches[1].end(), std::inserter(result, result.begin()));
    
    if( result.size() )
    {
#ifdef DEBUG
        std::cout << "Found " << result.size() << " matches\n";
#endif
        const CompleteSolution * answer = *result.begin();
        resp.success = true;
        resp.exactMatch = true;
        resp.problemDescription = answer->desc;
        resp.maxValue = answer->sol.maxValue;
        resp.location = answer->sol.maxValueLocation;
        resp.sol = new Solution(answer->sol);
    }
    else {
        resp.success = false;
    }
}

static void queryApproximately(const ProblemDescription& desc, QueryResponse& resp)
{
    resp.success = false;
}
