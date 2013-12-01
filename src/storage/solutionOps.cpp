#include <fstream>
#include <Iostream>
#include <map>
#include <boost/filesystem.hpp>

#include "common/problem.h"
#include "common/protocol.h"
#include "common/solution.h"

#include "solutionOps.h"

class CompleteSolution
{
public:
    ProblemDescription desc;
    Solution sol;
};

static const boost::filesystem::path solutionRoot("solutions");

// This map owns the solutions
std::map<ProblemID, CompleteSolution*> solutionById;

// TODO other indexes of solutions...

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
    
    // genome root exists, so find the genomes in it
    boost::filesystem::directory_iterator dir_end = boost::filesystem::directory_iterator();
    for( boost::filesystem::directory_iterator iter = boost::filesystem::directory_iterator(solutionRoot);
        iter != dir_end;
        ++iter )
    {
        std::string name = iter->path().filename().generic_string<std::string>();
        std::ifstream input(iter->path().generic_string<std::string>().c_str());
        
        CompleteSolution * sol = new CompleteSolution;
        readProblemDescription(input, sol->desc);
        readSolution(input, sol->sol);
    }
}

void insertSolution(const ProblemDescription& prob, const Solution& sol)
{
    
}
