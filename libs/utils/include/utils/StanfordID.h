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
    // constructor for our StudentID
    StanfordID(std::string name, std::string sunet, int idNumber);
    // method to get name, sunet, and idNumber, respectively
    std::string getName();
    std::string getSunet();
    int getID();

     ~StanfordID(); // Declare destructor
};