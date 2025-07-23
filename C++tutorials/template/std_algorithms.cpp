
#include <iostream>
#include <vector>
#include <algorithm>

void run_find(){
    std::vector<int> v = {3, 1, 4, 1, 5, 9, 2, 6, 5, 3, 5};
    auto it1= std::find(v.begin(), v.end(), 8);
    std::cout << *it1 << std::endl;
}

int main(){
    run_find();
    return 0;

}