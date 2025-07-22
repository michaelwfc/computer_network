#include <iostream>
#include <utility>
#include <cmath>

// A pair is a struct with two fields
// std::pair is a template. You can use any type inside it; type goes in the <>.
using Zeros = std::pair<double, double>;
using Solution = std::pair<bool, Zeros>;

Solution quadratic(double a, double b, double c)
{
    double discriminant = b * b - 4 * a * c;
    if (discriminant < 0)
    {
        return {false, {0, 0}};
    }
    else
    {
        return {true, {(-b + sqrt(discriminant)) / (2 * a), (-b - sqrt(discriminant)) / (2 * a)}};
    }
}


// Typing these types out is a pain...
int main()
{
    int a, b, c;
    std::cin >> a >> b >> c;
    
    std::pair<bool, std::pair<double, double>> result = quadratic(a, b, c);
    bool found = result.first;
    if (found)
    {
        std::pair<double, double> solutions = result.second;
        std::cout << solutions.first << solutions.second << std::endl;
    }
    else
    {
        std::cout << "No solutions found !" << std::endl;
    }
}

/**
 * auto does not mean that the variable doesn’t have a type.
It means that the type is deduced by the compiler.
 */
int autoExamples()
{
    // What types are these?
    //Answers: int, double, char, char* (a C string), std::pair<int, int>
    auto a = 3;
    auto b = 4.3;
    auto c = 'X';
    auto d = "Hello";
    auto e = std::make_pair(3, 3);
    return 0;
}


int main()
{
    auto [success, zeros] = quadratic(1, 2, 1);
    if (success)
    {

        std::cout << "Zeros: " << zeros.first << ", " << zeros.second << std::endl;
    }
    else
    {
        std::cout << "No real zeros" << std::endl;
    }
    return 0;
}