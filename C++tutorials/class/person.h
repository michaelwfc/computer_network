#include <iostream>

class Person
{
protected:
    std::string name;

public:
    Person(const std::string &name) : name(name) {}
    std::string getName();
}