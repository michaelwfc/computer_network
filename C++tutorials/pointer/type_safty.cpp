#include <iostream>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <optional>
#include "printUtils.h"

using namespace std;

void removeOddsFromEndV1(vector<int> &vec)
{
    // But in reality, there either may or may not be a “last element” in a vector
    while (vec.back() % 2 == 1)
    {
        vec.pop_back();
    }
}
void removeOddsFromEndV2(vector<int> &vec)
{
    while (!vec.empty() && vec.back() % 2 == 1)
    {
        vec.pop_back();
    }
}

template <typename T>
T &my_back(vector<T> &vec)
{
    if (vec.empty())
        throw out_of_range("vector is empty");
    return *(vec.begin() + vec.size() - 1);
}


/**
 * You can guarantee the behavior of your programs by using a strict type system!
 * std::optional is a tool that could make this happen: you can return either a value or nothing

 * std::optional types have a:
     .value() method: returns the contained value or throws bad_optional_access error
     .value_or(valueType val) returns the contained value or default value, parameter val
     .has_value() returns true if contained value exists, false otherwise

     .and_then(function f) : returns the result of calling f(value) if contained value exists, otherwise null_opt (f must return optional)
     .transform(function f): returns the result of calling f(value) if contained value exists, otherwise null_opt (f must return optional<valueType>)
     .or_else(function f) : returns value if it exists, otherwise returns result of calling f

 * 
 */
void run_optional()
{

    optional<int> num1 = {};
    num1 = 1;


    // cannot convert ‘num1’ (type ‘std::optional<int>’) to type ‘const unsigned char*’
    // cout << "num1 value" << num1 << endl;

    cout << "num1 value" << num1.value() << endl;
    num1 = nullopt;

    // 试图访问一个为空 (nullopt) 的 optional 的值，这会抛出 std::bad_optional_access 异常
    // cout << "num1 value" << num1 << endl;

    if (num1.has_value())
    {
        cout << "num1 value: " << num1.value() << endl;
    }
    else
    {
        
        cout << "num1 is empty" << endl;
    }
}

int main()
{
    // vector<int> vec = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
    // removeOddsFromEndV1(vec);
    // printVector(vec);

    run_optional();

    return 0;
}