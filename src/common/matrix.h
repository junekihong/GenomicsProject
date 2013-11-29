#ifndef __MATRIX_H__
#define __MATRIX_H__

#include <vector>
#include <iostream>
#include <algorithm>
#include "location.h"
#include "pair_location_value.h"


// Matrix class. Represents the matrix of a (local alignment) dynamic programming problem. It will also solve itself if you call it.
class Matrix
{
    // Used by every constructor to malloc off the matrix space.
    void allocateMatrix();
    
public:
    int length;
    int width;
    
	int** matrix;
	
	Matrix();
	Matrix(int _length, int _width);
    Matrix(std::vector<int>& topNumbers, std::vector<int>& leftNumbers);
    ~Matrix();

    // This function will prepare the matrix by filling in the sides with the initial values. After initialization, we can perform local alignment.
    void initialize(std::vector<int>& topNumbers, std::vector<int>& leftNumbers);

    // Performs local alignment on an initialized matrix. Returns the maximal value
    LocationValuePair localAlignment(std::vector<char>& topGenome, std::vector<char>& leftGenome);

};


std::ostream& operator<<(std::ostream& os, const Matrix& matrix);

#endif //__MATRIX_H__

