#ifndef DYNAMIC_PROGRAMMING_PROBLEM
#define DYNAMIC_PROGRAMMING_PROBLEM

#include <stdio.h>
#include <iostream>
#include <string>

using std::string;


class DynamicProgrammingProblem {
 public:
  string genome;
  string query;
  int scoringFunction[3][3];
  int* matrix;
  
  DynamicProgrammingProblem(string genomeString, string queryString){
    genome = genomeString;
    query = queryString;

    for (int i=0; i<3; i++){
      for(int j=0; j<3; j++){
	scoringFunction[i][j] = 0;
      }
    }

    printf("size of genome: %d\n",genome.size());
    printf("size of query: %d\n",query.size());

    
  }
  ~DynamicProgrammingProblem(){
  }

  void print(){
    // currently prints out the scoring function
    for (int i=0; i<3; i++){
      for(int j=0; j<3; j++){
	printf("%d\t", scoringFunction[i][j]);
      }
      printf("\n");
    }
  }


};


#endif


int main() {
//printf("hello swirled\n");
string genome = "GATTACAGATTACAGATTACA";
string query = "GAGA";
DynamicProgrammingProblem p1 = DynamicProgrammingProblem(genome, query);
p1.print();


}



