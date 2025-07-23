#include <iostream>
#include <vector>
#include "printUtils.h"

void get_address()
{
    int a = 10;
    int b = 20;
    int *p = &a;
    std::cout << "a: " << a << std::endl;
    std::cout << "p: " << p << std::endl;
    std::cout << "*p: " << *p << std::endl;

    std::cout << "after new assignment " << std::endl;
    *p = 20;
    std::cout << "a: " << a << std::endl;
    std::cout << "p: " << p << std::endl;
    std::cout << "*p: " << *p << std::endl;

    // Every T also has a pointer-to-const type, const T* (also written T const*)
    const int *p2 = &b;
    // *p2 = 30;  // Not allowed, `ptr` points to `const int`
    p2 = &a;
    std::cout << "p2: " << *p2 << std::endl;
    // std::cout << "b: " << b << std::endl;

    // if we wanted to prevent the pointer itself from being changed, we could use a const-pointer, e.g. T* const
}

void structred_pointer()
{
    std::pair<double, double> pair{10, 20};
    auto *ptr = &pair;
    double second = (*ptr).second; // or second = ptr->second;
    std::cout << "second: " << second << std::endl;
    // std::cout << "second: " << (*ptr).second << std::endl;
}

void pointer_to_heap()
{
    int *p1 = new int;     // default initialization
    int *p2 = new int(10); // value initialization:

    std::cout << "p1: " << *p1 << std::endl;
    std::cout << "p2: " << *p2 << std::endl;

    delete p1;
    delete p2;


    auto ptr1 = new std::pair<int, double>;
    auto ptr2 = new std::pair<int, double>{106, 3.14};
    auto ptr3 = new std::pair<int, double>();
    std::cout << "ptr1: " << ptr1->first << " " << ptr1->second << std::endl;
    std::cout << "ptr2: " << ptr2->first << " " << ptr2->second << std::endl;
    std::cout << "ptr3: " << ptr3->first << " " << ptr3->second << std::endl;

    delete ptr1;
    delete ptr2;
    delete ptr3;
}


void array_allocations(){
    double* p1= new double[5];
    double* p2= new double[5]();
    double* p3= new double[5]{1,2,3,4,5};
    printArray(p1,5);
    printArray(p2,5);
    printArray(p3,5);

    double* third =  p3+2;
    std::cout << "third: " << *third << std::endl;


    
}

int main()
{
    // get_address();
    // structred_pointer();
    // pointer_to_heap();
    array_allocations();
    return 0;
}