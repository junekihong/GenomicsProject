#include "../common/matrix.h"
#include "../common/location.h"
#include <vector>
#include <iostream>

int main()
{
    std::vector<int> top;
    std::vector<int> left;
    
    for(int i = 1; i <= 5; i++)
    {
        top.push_back(i);
        left.push_back(10 + i);

    }
    left.push_back(10 + 6);

    Matrix m = Matrix(top,left);

    
    std::cout << m << "\n"; 



    top.clear();
    left.clear();
    

    //example from the slides. Output seems to match results on the slides.
    // http://nbviewer.ipython.org/gist/BenLangmead/6994170
    char topList[] = {'T','A','T','A','T','G','C','G','G','C','G','T','T','T'};
    char leftList[] = {'G','G','T','A','T','G','C','T','G','G','C','G','C','T','A'};
    std::vector<char> topGenome = std::vector<char>(topList, topList + sizeof(topList)/sizeof(char));
    std::vector<char> leftGenome = std::vector<char>(leftList, leftList + sizeof(leftList)/sizeof(char));
    
    
    for(int i = 0; i< topGenome.size(); i++){
        std::cout << topGenome[i] << " ";
        
        top.push_back(0);
    }
    std::cout << "\n";


    for(int i = 0; i<leftGenome.size(); i++){
        std::cout << leftGenome[i] << " ";
        
        left.push_back(0);
    }
    std::cout << "\n";

    Matrix m2 = Matrix(top,left);
    
    std::cout <<m2 << "\n";
    

    
    LocationValuePair pair = m2.localAlignment(topGenome,leftGenome);
    int maxValue = pair.value;
    Location location = pair.location;

    std::cout <<m2 << "\n";

    std::cout << "max value: "<< maxValue << "\n";
    std::cout << "location: "<< location.row << " " << location.column << "\n";
    
}
