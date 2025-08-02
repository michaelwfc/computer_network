#include <string>

// An example of type aliasing
using String = std::string;


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