#ifndef __GENOME_OPS_H__
#define __GENOME_OPS_H__

#include <map>
#include <string>
#include <vector>

#include <msgpack.hpp>

void initializeGenomeSystem();
void createGenome(const std::string& name, unsigned length);
void addGenomeData(const std::string& genome, unsigned startIndex, const std::vector<unsigned char>& data);
const GenomeInfo& getGenomeInfo(const std::string& name);
void getGenomeData(const std::string& name, unsigned index, unsigned length, std::vector<unsigned char>& data);

extern std::map<std::string, GenomeInfo> genomes;

#endif /* __GENOME_OPS_H__ */
