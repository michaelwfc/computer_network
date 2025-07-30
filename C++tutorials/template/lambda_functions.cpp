#include <iostream>

auto func = [](int a, int b) { return a + b; };
int main(){
    auto result= func(1, 2);
    std::cout << result << std::endl;
    return 0;
}