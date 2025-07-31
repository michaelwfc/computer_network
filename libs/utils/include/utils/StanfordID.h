#include <string>

// An example of type aliasing
using String = std::string;

/**
 * 6 special member functions
● Default constructor: T() : Object created with no parameters, no member variables instantiated
● Destructor: ~T() ：Object destroyed when it is out of scope.
● Copy constructor: T(const T&) ： Object created as a copy of existing object (member variable-wise)
● Copy assignment operator: T& operator=(const T&) ：  Existing object replaced as a copy of another existing object
● Move constructor: T(T&&)
● Move assignment operator: T& operator=(T&&)
 */
class StanfordID
{
private:
    String name;
    String sunet;
    int idNumber;

public:
    
    StanfordID(std::string name, std::string sunet, int idNumber);
    

    // copy constructor: Creates a new object as a member-wise copy of another
    StanfordID(const StanfordID &other);
   
    // Copy Assignment Operator Invocation: Assigns an already existing object to another
    // Note that here both objects are constructed before the use of the = operator
    StanfordID& operator=(const StanfordID &other);

    


    // method to get name, sunet, and idNumber, respectively
    std::string getName();
    std::string getSunet();
    int getID() const;

     ~StanfordID(); // Declare destructor

     bool operator< (const StanfordID &other) const;

     /**
      * The friend keyword allows non-member functions or classes to access private information in another class!
      * In the header of the target class you declare the operator overload function as a friend
      */
     friend bool operator == (const StanfordID &lhs, const StanfordID &rhs);
     bool operator!=(const StanfordID &other) const;
};