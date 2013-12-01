#include "matrix.h"
#include <stdlib.h>
#include <cstring>

Matrix::Matrix()
: length(0), width(0), matrix(NULL)
{
    allocateMatrix();
}
  
Matrix::Matrix(int _length, int _width)
: length(_length), width(_width), matrix(NULL)
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

void Matrix::allocateMatrix()
{
    matrix = (int**)calloc(width + 1, sizeof(int*));
    for(int i = 0; i <= width; i++)
    {
        matrix[i] = (int*)calloc(length + 1, sizeof(int));
    }    
}

Matrix::~Matrix()
{
    for(int i = 0; i < width; i++)
    {
        free(matrix[i]);
    }
    free(matrix);
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
    if( matrix )
        this->~Matrix();
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


LocationValuePair Matrix::localAlignment(std::vector<char>& topGenome, std::vector<char>& leftGenome)
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
    for( unsigned i = 0; i <= width; ++i )
    {
        memcpy(matrix[i], other.matrix[i], sizeof(int) * (length + 1));
    }
    return *this;
}

