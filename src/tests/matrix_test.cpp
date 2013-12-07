#include <fstream>
#include <iostream>
#include <vector>
#include "common/matrix.h"
#include "common/location.h"
#include "../client/fasta.h"

int main(int argc, const char* argv[2])
{
    if( argc != 3 ) {
        std::cout << "Usage: matrix_test <path to genome> <path to genome>\n";
        return -1;
    }
    
    /*for(int i = 1; i <= 5; i++)
    {
        top.push_back(i);
        left.push_back(10 + i);

    }
    left.push_back(10 + 6);

    Matrix m = Matrix(top,left);

    
    std::cout << m << "\n"; 

    top.clear();
    left.clear();*/
    

    //example from the slides. Output seems to match results on the slides.
    // http://nbviewer.ipython.org/gist/BenLangmead/6994170
    //char topList[] = {'T','A','T','A','T','G','C','G','G','C','G','T','T','T'};
    //char leftList[] = {'G','G','T','A','T','G','C','T','G','G','C','G','C','T','A'};
    std::ifstream input(argv[1]);
    std::string topString = readFastaString(input);
    input.close();
    std::vector<unsigned char> topGenome(topString.begin(), topString.end());
    input.open(argv[2]);
    std::string leftString = readFastaString(input);
    input.close();
    std::vector<unsigned char> leftGenome(leftString.begin(), leftString.end());
    
    std::vector<int> top(topGenome.size());
    std::vector<int> left(leftGenome.size());
    
    /*for(int i = 0; i< topGenome.size(); i++){
        //std::cout << topGenome[i] << " ";
        
        top.push_back(0);
    }
    std::cout << "\n";


    for(int i = 0; i<leftGenome.size(); i++){
        //std::cout << leftGenome[i] << " ";
        
        left.push_back(0);
    }
    std::cout << "\n";*/

    Matrix m2 = Matrix(top,left);
    
    //std::cout <<m2 << "\n";
    

    
    LocationValuePair pair = m2.localAlignment(topGenome,leftGenome);
    int maxValue = pair.value;
    Location location = pair.location;

    //std::cout <<m2 << "\n";

    std::cout << "max value: "<< maxValue << "\n";
    std::cout << "location: "<< location.row << " " << location.column << "\n";
    
}
