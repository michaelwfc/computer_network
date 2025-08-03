#ifndef PRINT_UTILS_H
#define PRINT_UTILS_H

#include <iostream>
#include <vector>
#include <deque>

void printVector(std::vector<int> v);

void printArray(double *arr, int size);


// write a template function to print a deque
template <typename T>
void printDeque(std::deque<T> v){
    for (auto i : v)
    {
        std::cout << i << " ";
    }
    std::cout << std::endl;
}

#endif // PRINT_UTILS_H