#include <iostream>
#include "hello.hh" // Note: We will configure include paths in CMake

int main() {
  Hello hello_object;
  std::cout << hello_object.say_hello("michael") << std::endl;
}