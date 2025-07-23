class Shape
{
public:
    // Pure virtual function: it is instantiated in the base class but overwritten in the subclass
    virtual double area() const = 0;
};


// Here we declare the Circle class which inherits from the Shape class
class Circle : public Shape
{
    //Another pro of inheritance is the encapsulation of class variables.
private:
    double radius;

public:
    //constructor using list initialization construction
    Circle(double r) : radius(r) {}

    //Here we are overwriting the base class function area() for a circle
    double area() const {
        return 3.14 * radius * radius;
    };
};


class Rectangle : public Shape
{
private:
    double width;
    double height;

public:
    Rectangle(double w, double h) : width(w), height(h) {}

    double area() const {
        return width * height;
    };
};