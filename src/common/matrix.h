#ifndef __MATRIX_H__
#define __MATRIX_H__

class Matrix
{
    int length;
    int width;

	public:
	int** matrix;
	
	Matrix();
	Matrix(int _length, int _width);
    ~Matrix();

};


#endif //__MATRIX_H__

