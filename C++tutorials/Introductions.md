# C++
- [CS106L: Standard C++ Programming](http://web.stanford.edu/class/cs106l/)
- [cs106l.github.io/textbook](https://cs106l.github.io/textbook/)
- [cs106l-lecture-code/](https://github.com/cs106l/cs106l-lecture-code/tree/main/lecture02)
- [C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines)
- [C++ standard library](https://en.cppreference.com/w/cpp/standard_library)
- [Vedio-course-2019+2020](https://www.bilibili.com/video/BV1Fz421q7oh?spm_id_from=333.788.videopod.sections&vd_source=b3d4057adb36b9b243dc8d7a6fc41295)
- [CS自学指南](https://csdiy.wiki/%E7%BC%96%E7%A8%8B%E5%85%A5%E9%97%A8/cpp/CS106L/)

# C++ Design Philosophy

[C++ Design Philosophy](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#S-philosophy)

In 1983, the beginnings of C++ were created by Danish computer scientist Bjarne Stroustrup

- Do not waste time or space(high performance)
- Allow the programmer full control, responsibility, and choice.
- Express ideas and intent directly in code.
- Enforce safety at compile time whenever possible.
- Compartmentalize messy constructs.

C++ is a compiled language


# Types

A type refers to the “category” of a variable

C++ comes with built-in types:
- char: 8-bit character
- int: 32-bit signed integer
- double: 64-bit floating point number
- string: a sequence of characters
- bool: true or false
- size_t: unsigned integer type used for sizes

## C++ is a statically typed language

Why static typing?

- Better performance
- Easier to understand and reason about
- Better error checking


## Aside: Function Overloading

Defining two functions with the same name but different parameters

# std : The C++ Standard Library

- [the official standard](https://en.cppreference.com/w/)

#include from the C++ Standard Library to use built-in types
And use the std:: prefix too!




Built-in types, functions, and more provided by C++
You need to #include the relevant file
- #include <string> → std::string
- #include <utility> → std::pair
- #include <iostream> → std::cout, std::endl
  
We prefix standard library names with std::
If we write using namespace std; we don’t have to, but this is considered bad style as it can introduce ambiguity
(What would happen if we defined our own string?)

# Quality of life features to improve your code

- using creates type aliases
- auto infers the type of a variable


# C++ files

|        | Header File (.h) | Source File (.cpp)|
| ------ | --------------- | ------------------|
| Purpose | Defines the interface Implements class functions |
| Contains | Function prototypes, class  declarations, type definitions,  macros,constants | Function implementations, executable code|
| Access  | This is shared across source files | Is compiled into an object file|
| Example  | void someFunction();  |void someFunction(){...};|

## Types of inheritance
| Type  | public  | protected | private |
| ----- | ------- | --------- | ------- |
|Example | class B： public A{...} | class B: protected A{...} |  class B: private A{...} | 
|Public Members | Are public in the derived class | Protected in the derived class | Privated in the derived class |
|Protected Members | Protected in the derived class | Protected in the derived class | Privated in the derived class |
|Private Members |  Not accessible in derived class |Not accessible in derived class | Not accessible in derived class |