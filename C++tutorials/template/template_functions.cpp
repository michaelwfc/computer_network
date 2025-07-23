
/**
 * Template functions
 * lets you declare functions that can accept different types as parameters!
 */

#include <iostream>
#include <vector>

template <typename T>

// T min(T a, T b){
//     return a < b ? a : b;
// }

T min(const T& a, const T& b)
{
    return a < b ? a : b;
}   

template <typename DataType>
int count(const std::vector<DataType>& v, const DataType& x){
    int count = 0;
    for (auto i : v){
        if (i == x)
            count++;
    }
    return count;
}

int main(){ 
    auto min_int = min(1, 2);
    std::cout <<"min_int:" << min_int << std::endl;

    auto min_double = min(1.1, 2.2);
    std::cout <<"min_double:" << min_double << std::endl;

    auto min_string = min("hello", "world");
    std::cout <<"min_string:" << min_string << std::endl;
}