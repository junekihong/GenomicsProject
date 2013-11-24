#ifndef __PROBLEM_H__
#define __PROBLEM_H__

#include <vector>

class ProblemID
{
  unsigned long idnum;
  
  static unsigned long next_problem_id;
  
 public:
  ProblemID()
  : idnum(next_problem_id)
  {
    next_problem_id += 1;
  }
};

class ProblemDescription
{
    public:
    ProblemID id;
    std::vector<int> top_numbers;
    std::vector<int> left_numbers;
    std::vector<char> top_genome;
    std::vector<char> left_genome;
};

#endif // __PROBLEM_H__
