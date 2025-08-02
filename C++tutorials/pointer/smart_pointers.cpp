/**
 * How to we ensure that we properly release resources in the case that we have an exception?
 * RAII: Resource Acquisition is Initialization
 * So what is RAII?
    ● All resources used by a class should be acquired in the constructor!
    ● All resources that are used by a class should be released in the destructor.

There are three types of RAII-compliant pointers:
● std::unique_ptr    Uniquely owns its resource, can’t be copied,  When it goes out of scope, it automatically deletes the object it owns (no need to delete manually).
● std::shared_ptr    Can make copies, destructed when the underlying memory goes out of scope
● std::weak_ptr      A class of pointers designed to mitigate circular dependencies  More on these in a bit

Always use std::make_unique<T> and std::make_shared<T>

 */

#include <iostream>
#include <memory>

//  using namespace std;


// Example: Managing a resource without manual delete
void raw_example() {
    int* ptr = new int(42);
    std::cout << *ptr << std::endl;
    delete ptr;  // forgot? -> memory leak
}

void smart_example() {
    std::unique_ptr<int> ptr = std::make_unique<int>(42);
    std::cout << *ptr << std::endl;
    // No need to delete: automatically freed when `ptr` goes out of scope
}


/**
 * Real-World Use Case: Resource ownership in class
 * Copying is disabled by default for unique_ptr, You can allow move semantics instead
 * 
 * No need to manually new[] or delete[].
 * Prevents ownership bugs like double frees.
 * Encourages RAII (Resource Acquisition Is Initialization).

 */
   // 
    // 
class MyClass {
private:
    std::unique_ptr<int[]> buffer;

public:
    MyClass(size_t size) : buffer(std::make_unique<int[]>(size)) {
        std::cout << "Buffer allocated of size " << size << std::endl;
    }

    void set(int index, int value) {
        buffer[index] = value;
    }

    int get(int index) const {
        return buffer[index];
    }

 
};

void test_class() {
    MyClass obj(10);
    obj.set(0, 99);
    std::cout << obj.get(0) << std::endl;
}



 int main()
 { 
 }
