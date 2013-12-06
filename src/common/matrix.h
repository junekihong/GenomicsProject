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
    
    //MSGPACK_DEFINE(length, width, matrix);
    template<typename Packer>
    void msgpack_pack(Packer& pk) const
    {
        pk.pack_array(3);
        pk.pack(length);
        pk.pack(width);
        pk.pack_array(width + 1);
        for( int i = 0; i <= width; ++i ) {
            pk.pack_array(length + 1);
            for( int j = 0; j <= length; ++j ) {
                pk.pack(matrix[i][j]);
            }
        }
    }

	void msgpack_unpack(msgpack::object o)
	{
		if( o.type != msgpack::type::ARRAY )
			throw msgpack::type_error();
		const size_t size = o.via.array.size;
		if( size != 3 ) throw msgpack::type_error();
		o.via.array.ptr[0].convert(&length);
		o.via.array.ptr[1].convert(&width);
		msgpack::object& array = o.via.array.ptr[2];
		std::cout << "array has length " << array.via.array.size << "\n";
		std::cout << "length = " << length << "\n";
		std::cout << "width = " << width << "\n";
		matrix.resize(array.via.array.size);
		for( int i = 0; i <= width; ++i ) {
			msgpack::object& inner_array =  array.via.array.ptr[i];
			std::cout << "inner size = " << inner_array.via.array.size << "\n";
			matrix.at(i).resize(length+1);
			for( int j = 0; j <= length; ++j ) {
				inner_array.via.array.ptr[j].convert(&matrix.at(i).at(j));
			}
		}
	}

};


std::ostream& operator<<(std::ostream& os, const Matrix& matrix);

#endif //__MATRIX_H__

