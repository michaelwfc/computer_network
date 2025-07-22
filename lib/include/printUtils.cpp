#include <iostream>
#include <vector>
#include "printUtils.h"

void printVector(std::vector<int> v)
{
    for (auto i : v)
    {
        std::cout << i << " ";
    }
    std::cout << std::endl;
}


void printDeque(std::deque<int> v){
    for (auto i : v)
    {
        std::cout << i << " ";
    }
    std::cout << std::endl;
}