#include <iostream>
#include <ios>
#include "Vector.h"

template <typename T>
class MyClass
{
    const int _constant;
    int &_reference;

public:
    // Only way to initialize const and reference members
    // The constructor uses an initialization list to properly initialize these members, which is the only way to set const and reference members in C++. 
    MyClass(int value, int &ref) : _constant(value), _reference(ref) {}
};

int main()
{
    // This will use the default constructor (member initialization list version)
    Vector<int> vector;
    Vector<double> doubleVector;
    Vector<std::string> stringVector;

    for (int i = 0; i < 10; i++)
    {
        vector.push_back(i);
    }

    vector.at(0) = 100;
    vector.at(1) = 200;

    std::cout << "Size of Vector: " << vector.size() << std::endl;
    std::cout << "Contents of Vector: ";
    for (int i = 0; i < vector.size(); i++)
    {
        std::cout << vector.at(i) << " ";
    }
    std::cout << std::endl;



    // deep copy constructor
    Vector<int> vector2=vector;
    vector2.at(0) = 2000;
    std::cout << "copy constructor "<< std::endl;
    std::cout << "Contents of Vector: ";
    for (int i = 0; i < vector.size(); i++)
    {
        std::cout << vector.at(i) << " ";
    }
    std::cout << std::endl;

    std::cout << "Contents of vector2: "<< std::endl;
    for (int i = 0; i < vector2.size(); i++)
    {
        std::cout << vector2.at(i) << " ";
    }
    std::cout << std::endl;



    // deep copy assignment operator
    Vector<int> vector3;
    vector3 = vector;
    vector3.at(0) = 3000;
    std::cout << "copy assignment operator";
    std::cout << "Contents of Vector: ";
    for (int i = 0; i < vector.size(); i++)
    {
        std::cout << vector.at(i) << " ";
    }
    std::cout << std::endl;

    std::cout << "Contents of vector3: ";
    for (int i = 0; i < vector3.size(); i++)
    {
        std::cout << vector3.at(i) << " ";
    }
    std::cout << std::endl;

    return 0;
}