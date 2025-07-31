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
    Vector<int> vector2;
    vector2 =vector;
    vector2.at(0) = 1000;
    std::cout << "Contents of Vector: ";
    for (int i = 0; i < vector.size(); i++)
    {
        std::cout << vector.at(i) << " ";
    }
    std::cout << std::endl;

    std::cout << "Contents of Vector2: ";
    for (int i = 0; i < vector2.size(); i++)
    {
        std::cout << vector2.at(i) << " ";
    }
    std::cout << std::endl;

    return 0;
}