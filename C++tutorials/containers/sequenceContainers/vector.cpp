/**
 * Internally, a std::vector consists of an fixed-size array. 
 * The array is automatically resized when necessary
 * 
 */

#include <vector>
#include "printUtils.h"

int main(){
    // A static array in C++ is declared using the traditional C-style syntax:
    int arr[5] = {1,2,3,4,5};


    // Uniform initialization 
    std::vector <int> vec {1,2,3};
    printVector(vec);

    std::vector <int> vec2(5,3);
    printVector(vec2);

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