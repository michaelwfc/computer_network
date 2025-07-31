/**
 * ‼.cpp file‼ (implementation)
 *
 * In our .cpp file we need to use our class as our namespace when defining our member functions
 */

//  default constructor
// If we call our constructor without parameters we can set default ones!

#include <iostream>
#include <string>
#include "StanfordID.h"

// StanfordID::StanfordID()
// {
//     name = "John Appleseed";
//     sunet = "jappleseed";
//     idNumber = 00000001;
// }

// Parameterized Constructor
StanfordID::StanfordID(std::string name, std::string sunet, int idNumber)
{
    // Use this this keyword to disambiguate which ‘name’ you’re referring to.
    this->name = name;
    this->sunet = sunet;
    // We can now also enforce checks on the values that we initialize or modify our members to!
    if (idNumber > 0)
        this->idNumber = idNumber;
}

// list initialization constructor
// StanfordID::StanfordID(std::string name, std::string sunet, int idNumber) : name{name}, sunet{sunet}, idNumber{idNumber} {};

std::string StanfordID::getName()
{
    return this->name;
}
std::string StanfordID::getSunet()
{
    return this->sunet;
}
int StanfordID::getID() const
{
    return this->idNumber;
}

// void StanfordID::setName(std::string name)
// {
//     this->name = name;
// }
// void StanfordID::setSunet(std::string sunet)
// {
//     this->sunet = sunet;
// }
// void StanfordID::setID(int idNumber)
// {
//     if (idNumber >= 0)
//     {
//         this->idNumber = idNumber;
//     }
// }

// In our StanfordID class we are not dynamically allocating any data by using the new keyword
//  Nonetheless destructors are an important part of an object’s lifecycle.
//  The destructor is not explicitly called, it is automatically called when an object goes out of scope
StanfordID::~StanfordID()
{
    // free/deallocate any data here
    // delete [] my_array; // for illustration
}

/**
 * Member overloading
 * a. Declares the overloaded operator within the scope of your class
 * With member operator overloading we have access to this-> and the variables of the class
 */

bool StanfordID::operator<(const StanfordID &other) const
{
    return (this->idNumber < other.getID());
}

/**
 * 2. Non-member overloading
 * a. Declare the overloaded operator outside of classes
 * b. Define both the left and right hand objects as parameters
 *
 * The friend keyword allows non-member functions or classes to access private information in another class!

 */
bool operator==(const StanfordID &lhs, const StanfordID &rhs)
{
    // In this case the friend keyword is required
    return (lhs.idNumber == rhs.idNumber);
}

bool StanfordID::operator!=(const StanfordID &other) const
{
    return !(*this == other);
}
