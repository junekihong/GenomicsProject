#ifndef __GENOME_OPS_H__
#define __GENOME_OPS_H__

#include <string>

class GenomeInfo
{
public:
    std::string name;
    unsigned length;
    
    GenomeInfo(std::string n, unsigned l)
        : name(n), length(l)
    { }
    
    GenomeInfo(const GenomeInfo& other)
        : name(other.name), length(other.length)
    { }
};

void initializeGenomeSystem();
void createGenome(const std::string& name, unsigned length);
void addGenomeData(const std::string& genome, unsigned startIndex, const std::vector<char> data);
const GenomeInfo& getGenomeInfo(const std::string& name);
void getGenomeData(const std::string& name, unsigned index, unsigned length, std::vector<char>& data);

#endif /* __GENOME_OPS_H__ */
