#ifndef __PROTOCOL_COMMON_H__
#define __PROTOCOL_COMMON_H__

#include <iostream>

#define PROBLEM_LIST_REQUEST_ID		3
#define PROBLEM_LIST_RESPONSE_ID	3

#define PROBLEM_CLAIM_REQUEST_ID    4
#define PROBLEM_CLAIM_RESPONSE_ID   4

#define SOLUTION_REPORT_ID          5

class ProblemDescription;



typedef int message_id_t;

void readProblemDescription(std::istream& socket, ProblemDescription& cur_prob);

#endif /* __PROTOCOL_COMMON_H__ */
