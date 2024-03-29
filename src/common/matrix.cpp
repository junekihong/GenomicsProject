#include "matrix.h"
#include <stdlib.h>
#include <cstring>

Matrix::Matrix()
: length(0), width(0), matrix()
{
    allocateMatrix();
}
  
Matrix::Matrix(int _length, int _width)
: length(_length), width(_width), matrix()
{
    allocateMatrix();
}

Matrix::Matrix(const std::vector<int>& topNumbers, const std::vector<int>& leftNumbers)
{
    length = static_cast<int>(topNumbers.size()); // FIXME cast to int silences warning
    width = static_cast<int>(leftNumbers.size());
    allocateMatrix();
    initialize(topNumbers, leftNumbers);
}

Matrix::Matrix(const Matrix& other)
: length(0), width(0), matrix()
{
    (*this) = other;
}

void Matrix::allocateMatrix()
{
    matrix.resize(width+1);
    for(int i = 0; i <= width; i++)
    {
#ifdef DEBUG
        std::cout << "Setting matrix row length to " << length + 1 << "\n";
#endif
        matrix[i].resize(length + 1);
    }    
}

Matrix::~Matrix()
{
}


void Matrix::initialize(const std::vector<int>& topNumbers, const std::vector<int>& leftNumbers)
{
    for(int i = 0; i < length; i++){
        matrix[0][i+1] = topNumbers[i];
    }
    for(int i = 0; i < width; i++){
        matrix[i+1][0] = leftNumbers[i];
    }
}

void Matrix::resize(int newLength, int newWidth)
{
    length = newLength;
    width = newWidth;
    allocateMatrix();
}

std::ostream& operator<<(std::ostream& os, const Matrix& matrix)
{
    os << matrix.getLength() << " " << matrix.getWidth() << "\n";
    for(int i = 0; i <= matrix.getWidth(); i++)
    {
        for(int j = 0; j <= matrix.getLength(); j++)
        {
            os << matrix.matrix[i][j] << "\t";
        }
        os << "\n";
    }
    return os;
}


LocationValuePair Matrix::localAlignment(std::vector<unsigned char>& topGenome, std::vector<unsigned char>& leftGenome)
{
    LocationValuePair pair;
    Location location;
    int max = 0;
    for(int i = 1; i <= width; i++)
    {
        int genomeIndex1 = i-1;
        for (int j = 1; j <= length; j++)
        {
            int genomeIndex2 = j-1;
            int V1 = matrix[i-1][j] - 6;
            int V2 = matrix[i][j-1] - 6;
            int V3 = matrix[i-1][j-1];
            char genome1 = leftGenome[genomeIndex1];
            char genome2 = topGenome[genomeIndex2];
            if(genome1 == genome2){
                V3 += 2;
            }
            else{
                V3 -= 4;
            }
            
            int values[] = {V1, V2, V3, 0};
            int value = *std::max_element(values, values+4);
            matrix[i][j] = value;

            // maintain the maximum value
            if(max < value){
                max = value;
                
                // NOTE. matrix is padded by an extra row and column in the beginning.
                // To get the corresponding index within the genomes, You will need to subtract each value by 1.
                location.row = i;
                location.column = j;
            }
        }
    }
    pair.value = max;
    pair.location = location;

    return pair;
}

Matrix& Matrix::operator=(const Matrix& other)
{
    resize(other.length, other.width);
    matrix = other.matrix;
    return *this;
}

