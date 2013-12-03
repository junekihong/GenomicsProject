#include <fstream>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

#include <boost/filesystem.hpp>

#include "genomeOps.h"

static const boost::filesystem::path genomeRoot("genomes");
std::map<std::string, GenomeInfo> genomes;

void initializeGenomeSystem()
{
    
    if( !boost::filesystem::exists(genomeRoot) )
    {
        boost::filesystem::create_directory(genomeRoot);
        return;
    }
    
    if( !boost::filesystem::is_directory(genomeRoot) )
    {
        throw std::runtime_error("file \"" + genomeRoot.generic_string<std::string>() + "\" exists, but cannot"
                                 " be used as the genome root because it is not a directory");
    }
    
    // genome root exists, so find the genomes in it
    boost::filesystem::directory_iterator dir_end = boost::filesystem::directory_iterator();
    for( boost::filesystem::directory_iterator iter = boost::filesystem::directory_iterator(genomeRoot);
        iter != dir_end;
        ++iter )
    {
        std::string name = iter->path().filename().generic_string<std::string>();
        unsigned length = static_cast<unsigned>(boost::filesystem::file_size(*iter)); // TODO loses precision
        genomes.insert(std::pair<std::string, GenomeInfo>(name, GenomeInfo(name, length)));
    }
    
    std::cout << "Genome list (" << genomes.size() << ") :\n";
    for( std::map<std::string, GenomeInfo>::iterator iter = genomes.begin(); iter != genomes.end(); ++iter)
    {
        std::cout << iter->first << "\n";
    }
}

void createGenome(const std::string& name, unsigned length)
{
    genomes.insert(std::pair<std::string, GenomeInfo>(name, GenomeInfo(name, length)));
    std::ofstream strm((genomeRoot / name).generic_string<std::string>().c_str());
}

void addGenomeData(const std::string& genome, unsigned startIndex, const std::vector<char> data)
{
    std::ofstream strm((genomeRoot / genome).generic_string<std::string>().c_str());
    strm.seekp(startIndex);
    strm.write(data.data(), data.size());
}

const GenomeInfo& getGenomeInfo(const std::string& name)
{
    // TODO this without both lookups
    if( !genomes.count(name) ) {
        throw std::runtime_error("Couldn't find genome info on " + name);
    }
    
    return genomes.at(name);
}

void getGenomeData(const std::string& name, unsigned index, unsigned length, std::vector<char>& data)
{
    std::ifstream strm((genomeRoot / name).generic_string<std::string>().c_str());
    if( !strm )
        throw std::runtime_error("Error opening the file for genome " + name);
    data.resize(length);
    strm.seekg(index, std::ios::beg);
    strm.read(data.data(), length);
#ifdef DEBUG
    std::cout << "read data from index " << index << " in file " << (genomeRoot / name).generic_string<std::string>().c_str() << ":\n" << std::string(data.begin(), data.end()) << "\n";
#endif
}
