#include "scheduling.h"
#include <stdlib.h>
#include <cstdlib>




scheduler::Problem::Problem()
{
    right = NULL;
    down = NULL;
    right_down = NULL;
    requestor = NULL;

    first = NULL;

    left = false;
    up = false;
    left_up = false;
    
    job = NULL;
    solution = Solution();
}

scheduler::Job::Job()
{
    client = NULL;
    finished = false;
}

scheduler::Job::Job(std::vector<unsigned char> genome1, std::vector<unsigned char> genome2, LeaderClientProtocol* requestor, ProblemID problemNumber, int divisionConstant)
{
    client = requestor;
    finished = false;

    subproblemMatrix = std::vector<std::vector<Problem> >();
    for(int i = 0; i < divisionConstant; i++) {
        subproblemMatrix.push_back(std::vector<scheduler::Problem>());
        for(int j = 0; j < divisionConstant; j++){
            subproblemMatrix[i].push_back(scheduler::Problem());
        }
    }
    
    int firstSublength = genome1.size() / divisionConstant;
    int secondSublength = genome2.size() / divisionConstant;

    for(int j = 0; j < divisionConstant; j++)
    {
        int firstIndex = j * firstSublength;
        unsigned int offset = firstIndex + firstSublength;
        if(offset > genome1.size()){
            offset = genome1.size();
        }
        std::vector<unsigned char> firstChunk = std::vector<unsigned char>(genome1.begin() + firstIndex, genome1.begin() + offset);
        
        for(int i = 0; i < divisionConstant; i++)
        {
            int secondIndex = i * secondSublength;
            unsigned int offset2 = secondIndex + secondSublength;
            if(offset2 > genome2.size()){
                offset2 = genome2.size();
            }
            std::vector<unsigned char> secondChunk = std::vector<unsigned char>(genome2.begin() + secondIndex, genome2.begin() + offset2);
            
            subproblemMatrix[i][j].top_genome = firstChunk;
            subproblemMatrix[i][j].left_genome = secondChunk;
            subproblemMatrix[i][j].requestor = requestor;


            // QUESTION. problemNumber will internally keep state? Even after this constructor?
            subproblemMatrix[i][j].problemID = problemNumber;
            problemNumber.increment();
            problemMap[subproblemMatrix[i][j].problemID] = subproblemMatrix[i][j];

            subproblemMatrix[i][j].job = this;
            
            if(i > 0) {
                subproblemMatrix[i-1][j].down = &subproblemMatrix[i][j];
            }
            if(j > 0) {
                subproblemMatrix[i][j-1].right = &subproblemMatrix[i][j];
            }
            if(i > 0 && j > 0) {
                subproblemMatrix[i-1][j-1].right_down = &subproblemMatrix[i][j];
            }
            if(i > 0 || j > 0) {
                subproblemMatrix[i][j].first = &subproblemMatrix[0][0];
            }
        }
    } 

    // Initialize all the top numbers. 
    for(int j = 0; j < divisionConstant; j++)
    {
        subproblemMatrix[0][j].up = true;
        for(unsigned int k=0; k< subproblemMatrix[0][j].top_genome.size(); k++){
            subproblemMatrix[0][j].top_numbers.push_back(0);
        }
    }
    
    // Initialize all the left numbers.   
    for(int i = 0; i < divisionConstant; i++)
    {
        subproblemMatrix[i][0].left = true;
        for(unsigned int k=0; k< subproblemMatrix[i][0].left_genome.size(); k++){
            subproblemMatrix[i][0].left_numbers.push_back(0);
        }
    }
    
    availableProblems.push_back(subproblemMatrix[0][0]);
}


std::vector<scheduler::Problem> scheduler::Job::getAvailableProblems()
{
    std::vector<Problem> availableProblems_copy = std::vector<Problem>(availableProblems);
    availableProblems.clear();

    return availableProblems_copy;
}




void scheduler::Job::update(ProblemID problemID, Solution solution)
{
    Problem currentChunk = problemMap[problemID];
    Matrix currentMatrix = solution.matrix;

    currentChunk.solution = solution;
    
    std::vector<int> lastRow;
    for(int i = 1; i <= currentMatrix.getLength(); i++)
    {
        lastRow.push_back(currentMatrix.matrix[currentMatrix.getWidth()][i]);
    }
    std::vector<int> lastCol;
    for(int i = 1; i <= currentMatrix.getWidth(); i++)
    {
        lastCol.push_back(currentMatrix.matrix[i][currentMatrix.getLength()]);
    }
    int lastValue = currentMatrix.matrix[currentMatrix.getWidth()][currentMatrix.getLength()];
    
    if(currentChunk.right != NULL)
    {
        currentChunk.right->left = true;
        currentChunk.right->left_numbers = lastCol;
        
        // If right chunk is ready, then we can put it into the problemList 
        if(currentChunk.right->left && currentChunk.right->up && currentChunk.right->left_up) {
            availableProblems.push_back(*(currentChunk.right));
        }
    }
    if(currentChunk.down != NULL)
    {
        currentChunk.down->up = true;
        currentChunk.down->top_numbers = lastRow;
        
        // If down chunk is ready, then we can put it into the problemList 
        if(currentChunk.down->left && currentChunk.down->up && currentChunk.down->left_up) {
            availableProblems.push_back(*(currentChunk.down));
        }
    }
    if(currentChunk.right_down != NULL)
    {
        currentChunk.right_down->left_up = true;
        currentChunk.right_down->corner = lastValue;

        // If right_down chunk is ready, then we can put it into the problemList 
        if(currentChunk.right_down->left && currentChunk.right_down->up && currentChunk.right_down->left_up)
        {
            availableProblems.push_back(*(currentChunk.right_down));
        }
    }
    
    // If we updated the last chunk in the chunkMatrix, then we are finished.
    if(currentChunk.right == NULL && currentChunk.down == NULL && currentChunk.right_down == NULL){
        finished = true;
    }
}



Solution scheduler::Job::combineChunks()
{
    Solution solution;
    Matrix matrix;

    scheduler::Problem firstProblem = subproblemMatrix[0][0];
    solution.id = firstProblem.solution.id;
    solution.maxValue = firstProblem.solution.maxValue;
    solution.maxValueLocation = firstProblem.solution.maxValueLocation;

    int length = 0;
    int width = 0;
    for(unsigned int i = 0; i < subproblemMatrix.size(); i++) {
        width += subproblemMatrix[i][0].solution.matrix.getWidth();
    }
    for(unsigned int j = 0; j < subproblemMatrix[0].size(); j++){
        length += subproblemMatrix[0][j].solution.matrix.getLength();
    }
    matrix = Matrix(length, width);

    int chunkLength = subproblemMatrix[0][0].solution.matrix.getLength();
    int chunkWidth = subproblemMatrix[0][0].solution.matrix.getWidth();

    for(unsigned int i = 0; i < subproblemMatrix.size(); i++)
    {
        std::vector<scheduler::Problem> row = subproblemMatrix[i];
        for(unsigned int j = 0; j < row.size(); j++)
        {
            scheduler::Problem problem = subproblemMatrix[i][j];
            matrixCopy(matrix, problem.solution.matrix, chunkLength*j, chunkWidth*i);
            if(problem.solution.maxValue > solution.maxValue)
            {
                solution.maxValue = problem.solution.maxValue;
                solution.maxValueLocation = problem.solution.maxValueLocation;
            }
        }
    }

    solution.matrix = matrix;
    return solution;
}


