#include "matrix.h"
#include <stdlib.h>

Matrix::Matrix()
{
    length = 0;
    width = 0;
    allocateMatrix();
}
  
Matrix::Matrix(int _length, int _width)
{
    length = _length;
    width = _width;
    allocateMatrix();
}

Matrix::Matrix(std::vector<int>& topNumbers, std::vector<int>& leftNumbers)
{
    length = topNumbers.size();
    width = leftNumbers.size();
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


void Matrix::initialize(std::vector<int>& topNumbers, std::vector<int>& leftNumbers)
{
    for(int i = 0; i < length; i++){
        matrix[0][i+1] = topNumbers[i];
    }
    for(int i = 0; i < width; i++){
        matrix[i+1][0] = leftNumbers[i];
    }
}


std::ostream& operator<<(std::ostream& os, const Matrix& matrix)
{
    os << matrix.length << " " << matrix.width << "\n";    
    for(int i = 0; i <= matrix.width; i++)
    {
        for(int j = 0; j <= matrix.length; j++)
        {
            os << matrix.matrix[i][j] << "\t";
        }
        os << "\n";
    }
    return os;
}


int Matrix::localAlignment(std::vector<char>& topGenome, std::vector<char>& leftGenome, Location& location)
{
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
    return max;
}
