/**
 * Classes have public and private sections!
 * A user can access the public stuff
 * But is restricted from accessing the private stuff
 *
 * Struct:  There are no direct access controls while using structs
 *
 * Class design
● A constructor
● Private member functions/variables
● Public member functions (interface for a user)
● Destructor
 *
 *
 */

#include <iostream>
#include <string>
#include "StanfordID.h"

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



int main()
{
    Person p1("Tom", 18);
    p1.printInfo();

    StanfordID id = StanfordID("Tom", "tom", 18);
    std::cout << "id: " << id.getID() << " name: " << id.getName() << " sunet: " << id.getSunet() << std::endl;

    return 0;
}