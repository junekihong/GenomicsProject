#ifndef __MATRIX_H__
#define __MATRIX_H__

#include <vector>
#include <iostream>
#include <algorithm>
#include "location.h"
#include "pair_location_value.h"

#include <msgpack.hpp>

// Matrix class. Represents the matrix of a (local alignment) dynamic programming problem. It will also solve itself if you call it.
// Data is stored in column-major order
class Matrix
{
    // Used by every constructor to malloc off the matrix space.
    void allocateMatrix();
    
    int length;
    int width;
    
    public:
    std::vector< std::vector<int> > matrix;
	
	Matrix();
	Matrix(int _length, int _width);
    Matrix(const std::vector<int>& topNumbers, const std::vector<int>& leftNumbers);
    Matrix(const Matrix& matrix);
    ~Matrix();

    // This function will prepare the matrix by filling in the sides with the initial values. After initialization, we can perform local alignment.
    void initialize(const std::vector<int>& topNumbers, const std::vector<int>& leftNumbers);

    // Performs local alignment on an initialized matrix. Returns the maximal value
    LocationValuePair localAlignment(std::vector<unsigned char>& topGenome, std::vector<unsigned char>& leftGenome);
    
    int getLength() const {
        return length;
    }
    
    int getWidth() const {
        return width;
    }
    
    void resize(int newLength, int newWidth);
    
    Matrix& operator=(const Matrix& other);
    
    MSGPACK_DEFINE(length, width, matrix);
};


std::ostream& operator<<(std::ostream& os, const Matrix& matrix);

#endif //__MATRIX_H__

