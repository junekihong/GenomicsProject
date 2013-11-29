#ifndef __MATRIX_H__
#define __MATRIX_H__

#include <vector>
#include <iostream>
#include <algorithm>
#include "location.h"

class Matrix
{
    void allocateMatrix();

public:
    int length;
    int width;
    
    
	int** matrix;
	
	Matrix();
	Matrix(int _length, int _width);
    Matrix(std::vector<int>& topNumbers, std::vector<int>& leftNumbers);
    ~Matrix();

    void initialize(std::vector<int>& topNumbers, std::vector<int>& leftNumbers);

    // return maximal value
    int localAlignment(std::vector<char>& topGenome, std::vector<char>& leftGenome, Location& location);

};

std::ostream& operator<<(std::ostream& os, const Matrix& matrix);

#endif //__MATRIX_H__

