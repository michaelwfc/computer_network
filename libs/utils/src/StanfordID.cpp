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
int StanfordID::getID()
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

//In our StanfordID class we are not dynamically allocating any data by using the new keyword
// Nonetheless destructors are an important part of an object’s lifecycle.
// The destructor is not explicitly called, it is automatically called when an object goes out of scope
StanfordID::~StanfordID() {
// free/deallocate any data here
// delete [] my_array; // for illustration
}