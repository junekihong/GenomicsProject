#ifndef __PROTOCOL_COMMON_H__
#define __PROTOCOL_COMMON_H__

#include <iostream>

#define ANNOUNCE_CLIENT             1
#define ANNOUNCE_LEADER             2
#define ANNOUNCE_WORKER             3
#define ANNOUNCE_STORAGE            4

#define PROBLEM_LIST_REQUEST_ID		5
#define PROBLEM_LIST_RESPONSE_ID	5

#define PROBLEM_CLAIM_REQUEST_ID    6
#define PROBLEM_CLAIM_RESPONSE_ID   6

#define SOLUTION_REPORT_ID          7

#define GENOME_UPLOAD_START_ID      8

#define GENOME_LIST_REQUEST_ID      9
#define GENOME_LIST_RESPONSE_ID     9

#define LOCAL_ALIGN_START_ID        10
#define LOCAL_ALIGN_FINISH_ID       11

#define BUFF_SIZE 32*1024

class ProblemDescription;

typedef int message_id_t;

void readProblemDescription(std::istream& socket, ProblemDescription& cur_prob);

#endif /* __PROTOCOL_COMMON_H__ */
