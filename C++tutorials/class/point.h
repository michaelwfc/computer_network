// Contains interface, declarations 

#include <iostream>

class Point
{
public:
    Point(int x, int y);
    ~Point();
    int getX();
    int getY();

private:
    int x;
    int y;
    std::string color;
};


