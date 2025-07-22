/**
 * $ g++ main.cpp –o main 	    # g++ is the compiler, outputs binary to main
 * $ ./main						# This actually runs our program
 * 
 */

#include <iostream>
#include <utility>
#include <cmath>
#include <string>  // required for std::string
// Option 2: Add using namespace std; (less preferred)

/**
 * A struct bundles named variables into a new type
 * 
 */
struct StanfordID {
	std::string name; 			// These are called fields
	std::string sunet; 		// Each has a name and type
	int idNumber;
};


StanfordID issueNewID() {
    StanfordID id;		// Initialize struct
    id.name = "Jacob Roberts-Baca"; // Access field with ‘.’
    id.sunet = "jtrb"; 					
    id.idNumber = 6504417;

    //Using list initialization
    StanfordID id { "Jacob Roberts-Baca", "jtrb", 6504417 };

	return id;
}


int main()
{
    // Order depends on field order in struct. ‘=‘ is optional
    StanfordID jrb = { "Jacob Roberts-Baca", "jtrb", 6504417 };
    StanfordID fi { "Fabio Ibanez", "fibanez", 6504418 };

    std::cout << "Jacob's ID number is " << jrb.idNumber << std::endl;
    std::cout << "Fabio's ID number is " << fi.idNumber << std::endl;
}