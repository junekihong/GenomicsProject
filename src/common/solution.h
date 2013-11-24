#ifndef __SOLUTION_H__
#define __SOLUTION_H__

#include <vector>
#include "common/location.h"
#include "common/matrix.h"
#include "common/problem.h"

/*
class SolutionID
{
  unsigned long idnum;
  static unsigned long next_solution_id;
  
 public:
  SolutionID();
  //: idnum(next_solution_id)
  //{
  //  next_solution_id += 1;
  //  }
};
*/


class SolutionCertificate
{
 public:
  unsigned long id;
  //ProblemID problemID;

  SolutionCertificate();
};



class Solution
{
 public:
  unsigned long id;
  //ProblemID problemID;

  int maxValue;
  Location maxValueLocation;
  Matrix matrix;
};



#endif // __SOLUTION_H__
