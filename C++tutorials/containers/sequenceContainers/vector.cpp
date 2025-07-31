/**
 * Internally, a std::vector consists of an fixed-size array. 
 * The array is automatically resized when necessary
 * 
 */
#include <iostream>
#include <vector>
#include <algorithm>
#include "printUtils.h"

int run_vector(){
    // A static array in C++ is declared using the traditional C-style syntax:
    int arr[5] = {1,2,3,4,5};

    /** Uniform initialization 
     * Uniform initialization is a C++11 feature that provides a consistent syntax for initializing objects using brace-initialization ({}).
     - Consistent Syntax: Uses curly braces {} for all types of initialization
     - Prevents Narrowing: Helps prevent implicit conversions that might lose data
     - Works Everywhere: Can be used for built-in types, user-defined types, arrays, and containers
     */ 

    std::vector <int> vec {1,2,3};
    printVector(vec);

    std::vector <int> vec2(5,3);
    printVector(vec2);

    // custom constructor with size
    std::vector<int> vec3(8);
    printVector(vec3);

    // Copy Constructor
    vector<int> vec5(vec2);

    //Initializer list is empty – empty vector via list initialization
    std::vector<int> vec6{};

    //List initialization
    std::vector<int> vec7{static_cast<int>(vec2.size() + vec6.size())};

    vec.push_back(4);
    printVector(vec);

    vec[0] =5;
    printVector(vec);

    vec.insert(vec.begin(), 6);
    printVector(vec);

    vec.insert(vec.begin()+2, 7);
    printVector(vec);

    vec.erase(vec.begin()+3);
    printVector(vec);


    return 0;
}

/**
 * use the standard algorithm std::find to find the first element in a vector that matches a given value
 * However, there's a logical error - since 8 doesn't exist in the vector, std::find returns v.end(), and dereferencing it (*it1) causes undefined behavior. 
 * 
 * v.end() returns an iterator that points to the position one past the last element of the vector v.
 * The vector v contains 11 elements with indices 0-10. Therefore, v.end() points to the position that would be at index 11 - one position past the last valid element.
 * his is part of the standard C++ iterator convention where begin() points to the first element and end() points to one position past the last element, 
 * allowing for idiomatic loop constructs like:
 * for (auto it = v.begin(); it != v.end(); ++it) { ... }
 */
void run_find(){
    std::vector<int> v = {3, 1, 4, 1, 5, 9, 2, 6, 5, 3, 5};
    auto it1= std::find(v.begin(), v.end(), 4);
    // Check if element was found before dereferencing
    if (it1 != v.end()) {
        std::cout << *it1 << std::endl;
    } else {
        std::cout << "Element not found" << std::endl;
    }
}

int main() {
    run_vector();
    // run_find();
    return 0;
}