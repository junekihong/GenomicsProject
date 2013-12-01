#include <sys/socket.h>

#include "protocol.h"
#include "problem.h"

void readMatrix(int socket, Matrix& mat, const std::string& err)
{
    int length, width;
    readItem(socket, length, "Error reading length of " + err);
    readItem(socket, width, "Error reading width of " + err);
    mat.resize(length, width);
    
    for( int i = 0; i <= mat.getWidth(); ++i ){
        recvfrom(socket, mat.matrix[i], (mat.getLength() + 1)*sizeof(int), 0, NULL, NULL);
    }
}

void sendMatrix(int socket, const Matrix& mat, const std::string& err)
{
    int length = mat.getLength();
    int width = mat.getWidth();
    sendItem(socket, length, "Error sending length of " + err);
    readItem(socket, width, "Error sending width of " + err);
    
    for( int i = 0; i <= mat.getWidth(); ++i ) {
        send(socket, mat.matrix[i], (mat.getLength() + 1)*sizeof(int), 0);
    }
}

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
