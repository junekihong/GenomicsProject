#include "protocol.h"
#include "problem.h"

void readProblemDescription(std::istream& socket, ProblemDescription& cur_prob)
{
    socket >> cur_prob.id.idnum;
    
    unsigned vec_length;
    socket >> vec_length;
    cur_prob.top_numbers.resize(vec_length);
    socket.read(reinterpret_cast<char*>(cur_prob.top_numbers.data()), vec_length * sizeof(std::vector<int>::value_type));
    cur_prob.top_genome.resize(vec_length);
    socket.read(reinterpret_cast<char*>(cur_prob.top_genome.data()), vec_length * sizeof(std::vector<char>::value_type));
    
    socket >> vec_length;
    cur_prob.left_numbers.resize(vec_length);
    socket.read(reinterpret_cast<char*>(cur_prob.left_numbers.data()), vec_length * sizeof(std::vector<int>::value_type));
    cur_prob.left_genome.resize(vec_length);
    socket.read(reinterpret_cast<char*>(cur_prob.left_genome.data()), vec_length * sizeof(std::vector<char>::value_type));

}