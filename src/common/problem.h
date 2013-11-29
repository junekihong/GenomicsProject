#ifndef __PROBLEM_H__
#define __PROBLEM_H__

#include <vector>

class ProblemID
{
    public:
	unsigned long idnum;
	
    private:
	static unsigned long next_problem_id; // This should only be on the leader
	
	public:
	ProblemID(unsigned long _id = 0)
        : idnum(_id)
	{ }
};

class ProblemDescription
{
	public:
	ProblemID problemID;
	std::vector<int> top_numbers;
	std::vector<int> left_numbers;
	std::vector<char> top_genome;
	std::vector<char> left_genome;
};

#endif // __PROBLEM_H__
