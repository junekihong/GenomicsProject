#include <sys/socket.h>

#include "protocol.h"
#include "problem.h"

void readProblemDescription(std::istream& socket, ProblemDescription& cur_prob)
{
    readItem(socket, cur_prob.problemID.idnum);
    readItem(socket, cur_prob.corner);
    readVector(socket, cur_prob.top_numbers);
    readVector(socket, cur_prob.left_numbers);
    readVector(socket, cur_prob.top_genome);
    readVector(socket, cur_prob.left_genome);
}

void sendProblemDescription(int socket, const ProblemDescription& cur_prob)
{
    sendItem(socket, cur_prob.problemID.idnum, "Error sending problem ID");
    sendItem(socket, cur_prob.corner, "Error sending problem corner element");
    sendVector(socket, cur_prob.top_numbers, "Error sending the top numbers of a problem");
    sendVector(socket, cur_prob.left_numbers, "Error sending the left numbers of a problem");
    sendVector(socket, cur_prob.top_genome, "Error sending the top genome of a problem");
    sendVector(socket, cur_prob.left_genome, "Error sending the left genome of a problem");
}
