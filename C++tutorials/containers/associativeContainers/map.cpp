#include <iostream>
#include <map>
#include <string>

void runMap()
{
    std::map<std::string, std::string> famous_people;
    famous_people = { {"Bjarne", "Stroustrup"},
                      {"Herb", "Sutter"},
                      {"Alexander", "Stepanov"} };
    for(const auto &[key, value] : famous_people){
        std::cout << key << ": " << value << std::endl;
    }
}


int main()
{
    runMap();
    return 0;
}