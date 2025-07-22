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