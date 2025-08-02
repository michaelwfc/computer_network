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
#include "Vector.h"


StanfordID min(const StanfordID &a, const StanfordID &b){
    // Compiler: “Hey, now I know what to do here!
    return a < b ? a : b;
}


int run_starfordid()
{

    // uniform init 
    StanfordID id0{"Alice","alice123",15};
    std::cout << id0.getName() << " " << id0.getID() << " " << id0.getSunet() << std::endl;

    
    StanfordID id1 = StanfordID("Tom", "tom", 18);
    std::cout << "id1 id: " << id1.getID() << " name: " << id1.getName() << " sunet: " << id1.getSunet() << std::endl;

    // copy constructor: Creates a new object as a member-wise copy of another
    StanfordID id4 = id1;
    std::cout << "id4 id: " << id4.getID() << " name: " << id4.getName() << " sunet: " << id4.getSunet() << std::endl;

    // Copy Assignment Operator Invocation: Assigns an already existing object to another
    // Note that here both objects are constructed before the use of the = operator
    StanfordID id5= StanfordID("Henry", "henry", 1);
    id5 = id1;
    std::cout << "id5 id: " << id5.getID() << " name: " << id5.getName() << " sunet: " << id5.getSunet() << std::endl;



    // test the operator overload
    StanfordID id2 = StanfordID("Jerry", "jerry", 19);
    if(id1<id2){
        std::cout << "id1 is smaller than id2" << std::endl;
    }else{
        std::cout << "id1 is larger than id2" << std::endl;
    }

    // test non-member operator overloading
    StanfordID id3 = StanfordID("Michale", "mm", 19);
    if(id2==id3){
        std::cout << "id2 is equal to id3" << std::endl;
    }else{
        std::cout << "id2 is not equal to id3" << std::endl;
    }

    // use operator to build function
    auto minID = min(id1,id2);
    std::cout << "minID is " << minID.getName() << std::endl;
    return 0;
}



int main() {
    run_starfordid();
    return 0;
}