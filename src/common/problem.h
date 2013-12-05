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

    void operator=(const ProblemID &x) {
        idnum = x.idnum;
    }

    bool operator==(const ProblemID &x) const
    {
        return (idnum == x.idnum);
    }
    
    bool operator<(const ProblemID& other) const {
        return idnum < other.idnum;
    }
    
    void increment() {
        idnum++;
    }
};

class ProblemDescription
{
	public:
	ProblemID problemID;
    // the int value to the upper-left of this problem chunk
    int corner;
    
	std::vector<int> top_numbers;
	std::vector<int> left_numbers;
	std::vector<char> top_genome;
	std::vector<char> left_genome;

    void operator=(const ProblemDescription &x)
    {
        problemID = x.problemID;
        corner = x.corner;
        top_numbers = x.top_numbers;
        left_numbers = x.left_numbers;
        top_genome = x.top_genome;
        left_genome = x.left_genome;
    }

    bool operator==(const ProblemDescription &x) const 
    {
        return (top_numbers == x.top_numbers) && (left_numbers == x.left_numbers) && (top_genome == x.top_genome) && (left_genome == x.left_genome);
    }
    
    bool operator<(const ProblemDescription& other) const
    {
        return problemID < other.problemID;
    }
};

    


#endif // __PROBLEM_H__
