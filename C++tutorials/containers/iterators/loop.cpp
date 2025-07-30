/**
 * iterator has ordering over elems
i.e. it always knows what the “next” element is
 *
 */
#include <iostream>
#include <vector>
#include <deque>
#include <map>
#include <set>

void loop_vector()
{
    std::cout << "loop_vector" << std::endl;
    std::vector<int> v = {1, 2, 3, 4, 5};
    for (size_t i = 0; i < v.size(); i++)
    {
        // In C++, the auto keyword tells the compiler to automatically deduce the type of a variable from its initializer. 
        // It helps make code cleaner and less error-prone, especially when dealing with long or complex types.
        // Avoids mistakes when types are complex or templated.
        // Useful in ranged-for loops and lambdas.
        const auto &elem = v[i];
        std::cout << elem << " ";
    }
    std::cout << std::endl;

    std::cout << "loop_vector with for-each loop" << std::endl;
    for(const auto &elem : v){
        std::cout << elem << " ";
    }
    std::cout << std::endl;

}



/**
 * s.begin(); // an iterator pointing to the first element
 * s.end() // one past the last element
 * elem++: increment; prefix operator is faster (why?)
 * *elem: dereference iter to get curr value
 */
void loop_set()
{
    std::cout << "loop_set" << std::endl;
    std::set<int> s = {5, 1, 3, 4, 2};
    for (auto elem = s.begin(); elem != s.end(); ++elem)
    {
        // std::cout << *elem << " ";
        const auto &elem_ref = *elem;
        std::cout << elem_ref << " ";
    }
    std::cout << std::endl;

    std::cout << "loop_set with for-each loop" << std::endl;
    for(const auto &elem : s){
        std::cout << elem << " ";
    }
    std::cout << std::endl;
}

/**
 *
 * elem= m.begin(): a copy of begin iterator
 * *elem: return {1,2}, dereferencing a std::map::iterator returns a std::pair
 * ++elem: incremented to next element
 */
void loop_map()
{
    std::cout << "loop_map" << std::endl;
    std::map<int, int> m = {{3, 4}, {5, 6}, {1, 2}};
    for (auto elem = m.begin(); elem != m.end(); ++elem)
    {
        // std::cout << elem->first << " " << elem->second << std::endl;
        std::cout << (*elem).first << " " << (*elem).second << std::endl;
    }
       
    std::cout << "loop_map with for-each loop" << std::endl;
    for(const auto &[key, value] : m){
        std::cout << key << " " << value << std::endl;
    }
    
}

int main()
{
    loop_vector();

    loop_set();
    loop_map();
    return 0;
}