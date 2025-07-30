#include <iostream>
#include <ios>
#include "Vector.h"

int main()
{
    Vector<int> vector;
    Vector<double> doubleVector;
    Vector<std::string> stringVector;

    for(int i = 0; i < 10; i++){
        vector.push_back(i);
    }

    vector.at(0) =100;
    vector.at(1) =200;

    std::cout << "Size of Vector: " << vector.size() << std::endl;
    std::cout << "Contents of Vector: ";
    for(int i = 0; i < vector.size(); i++){
        std::cout << vector.at(i) << " ";
    }
    std::cout << std::endl;

    return 0;
}