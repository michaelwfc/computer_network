#include <iostream>
#include <string>


class Person
{
private: // private member variables
    std::string name;
    int age;

public: // constructor
    Person(std::string name, int age)
    {
        this->name = name;
        this->age = age;
    }

    // public method
    void printInfo()
    {
        std::cout << "name: " << name << " age: " << age << std::endl;
    }
};
