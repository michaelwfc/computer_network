
/**
 * References : Allow us to alias variables
 * Const: Allow us to specify that a variable can’t be modified

 * If we’re working with a variable that takes up little space in memory (e.g. int, double), we don’t need to use a reference and can just copy the variable
 * If we need to alias the variable to modify it, we can use references 
 * If we don’t need to modify the variable, but it’s a big variable (e.g. std::vector), we can use const references
 * 
 */

#include <iostream>
#include <vector>
#include "printUtils.h"// lib/printUtils.h"

// void printVector(std::vector<int> v)
// {
//     for (auto i : v)
//     {
//         std::cout << i << " ";
//     }
//     std::cout << std::endl;
// }

/**
 * Pass-by-Reference
 * If we want to pass a variable from a calling function into another function and have it be modified then we need to pass it by reference.
 * 
 * @param: x inside doubleValue is an alias for myValue in main. This means it’s another name for the same variable!
 * A change to x is a change to myValue.
 */

int doubleValueByReference(int &x)
{
    x *= 2;
    return x;
}

int doubleValue(int x) {
  x *= 2;
  return x;
}

int run_func_argument_reference()
{
    int value = 5;
    std::cout << "*****Pass value by Copy " << std::endl;
    std::cout << "original value  = " << value << std::endl;  
    int result = doubleValue(value);
    std::cout << "value  = " << value << std::endl;
    std::cout << "result = " << result << std::endl;

    // The variable value is passed by reference into doubleValue.
    std::cout << "****Pass value by Reference "  << std::endl;
    std::cout << "original value  = " << value << std::endl;
    result = doubleValueByReference(value);
    std::cout << "value  = " << value << std::endl;
    std::cout << "result = " << result << std::endl;
    return 0;
}

void run_reference_for_vector()
{
    std::vector<int> original{1, 2, 3};
    std::vector<int> copy = original;     // If you don’t write &, C++ will make a copy by default!
    std::vector<int> &reference = original;
    original.push_back(6);
    copy.push_back(7);
    reference.push_back(8);
    printVector(original);
    printVector(copy);
    printVector(reference);

    reference=copy;
    copy.push_back(9);
    reference.push_back(10);
    printVector(original);
    printVector(copy);
    printVector(reference);
}


int& front(std::vector<int>& vec){ //the parameter must be a non-const reference
    return vec[0]; //return  a non-const reference to one of its elements!
}

// const int& front(const std::vector<int>& vec){ //the parameter must be a const reference
//     return vec[0]; //return a const reference to one of its elements!
// }

void run_update_front_element(){
    std::vector<int> vec = {1,2,3};
    front(vec) = 4;
    printVector(vec);
}


void run_constant_reference()
{
    std::vector<int> original = {1,2,3};
    const std::vector<int>c_original {7,8}; // a const variable
    std::vector<int> &reference = original; // a regular reference
    const std::vector<int> &c_reference = original; // a const reference: a  variables can be references
   
    // std::vector<int>&bad_ref =c_original;  // BAD - can't declare non-const ref to const vector
    original.push_back(4);
    // c_original.push_back(5); //bad -const variable can't be modified
    reference.push_back(6);
    // c_reference.push_back(7);   // bad
}

int main()
{
      run_func_argument_reference();
    // run_reference_for_vector();
    // run_update_front_element();

    return 0;
}