# C++ Refernce
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


# Most Important Ideas in Modern C++

Modern C++ (roughly C++11 → C++23) is not just “old C++ with extra syntax.”
It represents a major philosophical shift:

> From manual low-level programming → toward safer, expressive, zero-cost abstractions.

Below is a practical systems-programming-oriented overview.

| Feature                          | Big Idea                                  | Description                                     | Example                          | Why Important                             | Big Shift with Old C++                          |
| -------------------------------- | ----------------------------------------- | ----------------------------------------------- | -------------------------------- | ----------------------------------------- | ----------------------------------------------- |
| RAII                             | Resource lifetime tied to object lifetime | Resources automatically released in destructors | `std::lock_guard`, `std::vector` | Prevents leaks and cleanup bugs           | Old C++ relied heavily on manual `new/delete`   |
| Smart Pointers                   | Explicit ownership                        | Encode ownership in types                       | `unique_ptr`, `shared_ptr`       | Safer memory management                   | Raw pointers used everywhere before             |
| Move Semantics                   | Transfer ownership instead of copying     | Avoid expensive copies                          | `std::move(x)`                   | Huge performance improvement              | Old C++ copied objects constantly               |
| Rvalue References (`&&`)         | Distinguish temporary objects             | Enable move semantics/perfect forwarding        | `T&&`                            | Foundation of modern performance patterns | Did not exist pre-C++11                         |
| Rule of Zero                     | Let compiler manage special functions     | Avoid manual destructor/copy/move logic         | no custom destructor             | Fewer bugs                                | Old style manually wrote copy constructors      |
| `auto` Type Deduction            | Reduce verbosity                          | Compiler infers type                            | `auto x = vec.begin();`          | Cleaner generic code                      | Old C++ had extremely verbose types             |
| Range-based for                  | Safer iteration                           | Cleaner container traversal                     | `for(auto &x : vec)`             | Easier and less error-prone               | Old C++ used iterator boilerplate               |
| Lambdas                          | Inline anonymous functions                | Functional-style callbacks                      | `[](int x){ return x+1; }`       | Essential for async/concurrency/STL       | Old C++ required functor classes                |
| STL Containers                   | High-level data structures                | Standardized collections                        | `vector`, `map`, `deque`         | Safer/faster than manual arrays           | Old C++ often used raw arrays                   |
| STL Algorithms                   | Generic reusable algorithms               | Separate algorithm from container               | `std::sort()`                    | Powerful abstraction                      | Old style wrote loops manually                  |
| Templates                        | Compile-time generic programming          | Write type-independent code                     | `template<typename T>`           | Enables STL and zero-cost abstractions    | Older languages often used runtime polymorphism |
| constexpr                        | Compile-time computation                  | Run logic during compilation                    | `constexpr int f()`              | Faster and safer                          | Old C++ mostly runtime-only                     |
| `string_view`                    | Non-owning string view                    | Read strings without copying                    | `std::string_view sv`            | Major performance optimization            | Old code copied strings frequently              |
| `span`                           | Non-owning array view                     | Safer array slicing                             | `std::span<int>`                 | Prevents pointer/length mismatch          | Old C++ used raw pointer + size                 |
| Enum Class                       | Strongly typed enums                      | Avoid namespace pollution                       | `enum class Color`               | Safer APIs                                | Old enums implicitly converted to int           |
| Uniform Initialization           | Consistent initialization syntax          | One syntax for all objects                      | `Foo f{1,2}`                     | Avoids initialization confusion           | Old C++ had many inconsistent forms             |
| `nullptr`                        | Proper null pointer type                  | Type-safe null                                  | `nullptr`                        | Avoids overload ambiguity                 | Old C++ used integer `NULL`                     |
| `override` / `final`             | Safer inheritance                         | Compiler verifies virtual overrides             | `void foo() override`            | Prevents subtle polymorphism bugs         | Old C++ silently accepted mistakes              |
| Concurrency Library              | Standard threading support                | Portable multithreading                         | `std::thread`                    | Critical for modern systems               | Old C++ depended on platform APIs               |
| Futures / Async                  | Structured async execution                | Easier parallelism                              | `std::async()`                   | Simplifies concurrency                    | Old code used manual thread management          |
| Coroutines (C++20)               | Lightweight async programming             | Suspend/resume functions                        | `co_await`                       | Important for networking/IO               | Old async code used callback hell               |
| Concepts (C++20)                 | Constrain templates                       | Better template errors                          | `template<typename T> requires`  | Massive usability improvement             | Old templates produced unreadable errors        |
| Modules (C++20)                  | Better code organization                  | Replace textual includes                        | `import std;`                    | Faster builds, cleaner dependencies       | Old C++ relied on fragile headers               |
| Structured Bindings              | Tuple/object unpacking                    | Cleaner extraction syntax                       | `auto [a,b] = pair;`             | Improves readability                      | Old code used `.first/.second`                  |
| Variant / Optional               | Safer alternatives to raw unions/nulls    | Explicit state modeling                         | `std::optional<int>`             | Reduces invalid states                    | Old C++ used sentinel values                    |
| Type Inference + Templates       | Zero-cost abstraction                     | High-level syntax with low-level performance    | generic STL code                 | Core philosophy of modern C++             | Old abstraction often implied runtime overhead  |
| Zero-Cost Abstraction            | Abstraction without runtime penalty       | Compiler optimizes abstractions away            | iterators/templates              | Fundamental design principle              | Contrasts with many managed runtimes            |
| Ownership Semantics              | Ownership visible in API                  | Clarify who owns memory                         | `unique_ptr<T>` parameter        | Critical for systems safety               | Old C++ ownership often undocumented            |
| Immutability / const correctness | Restrict mutation                         | Compiler-enforced read-only behavior            | `const T&`                       | Prevents accidental mutation              | Old code often ignored const                    |
| Value Semantics                  | Prefer values over pointers               | Objects behave like values                      | returning `std::string`          | Simpler reasoning                         | Old C++ heavily pointer-oriented                |
| Header-only Generic Libraries    | Compile-time polymorphism                 | Templates replace runtime dispatch              | STL/Eigen                        | High performance                          | Old OOP relied heavily on virtual calls         |

---

## The Biggest Philosophical Shifts

### 1. From Manual Memory → Ownership Semantics

Old C++:

```cpp
Foo* p = new Foo();
delete p;
```

Modern C++:

```cpp
auto p = std::make_unique<Foo>();
```

Big shift:

```text
Ownership becomes explicit and automatic.
```

---

### 2. From Copying → Moving

Old C++:

```text
Performance often required pointer tricks.
```

Modern C++:

```cpp
std::move(x)
```

Big shift:

```text
Efficient transfer of resources without copying.
```

---

### 3. From Raw Loops → Generic Algorithms

Old C++:

```cpp
for(int i = 0; i < n; i++)
```

Modern C++:

```cpp
std::ranges::sort(vec);
```

Big shift:

```text
Describe WHAT, not HOW.
```

---

### 4. From Unsafe APIs → Type-Safe APIs

Old C++:

```cpp
NULL
char*
int enum
```

Modern C++:

```cpp
nullptr
string_view
enum class
optional<T>
```

Big shift:

```text
More bugs caught at compile time.
```

---

### 5. From Runtime Polymorphism → Compile-Time Polymorphism

Old style:

```cpp
virtual function
```

Modern style:

```cpp
templates + concepts
```

Big shift:

```text
High abstraction without runtime cost.
```

---

## The Most Important Concepts for Systems/Networking/GenAI Engineers

If you work on:

* networking
* databases
* game engines
* browsers
* AI infrastructure
* inference systems
* distributed systems

these are the highest ROI concepts:

| Priority | Concept                              |
| -------- | ------------------------------------ |
| 1        | RAII                                 |
| 2        | Ownership semantics                  |
| 3        | Move semantics                       |
| 4        | Smart pointers                       |
| 5        | const correctness                    |
| 6        | STL containers                       |
| 7        | Templates                            |
| 8        | Lambdas                              |
| 9        | string_view/span                     |
| 10       | Concurrency                          |
| 11       | Coroutines                           |
| 12       | Cache-friendly/value-oriented design |

---


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


# Special Member Functions

6 special member functions

- Default constructor: T() : Object created with no parameters, no member variables instantiated
- Destructor: ~T() ：Object destroyed when it is out of scope.
- Copy constructor: T(const T&) ： Object created as a copy of existing object (member variable-wise)
- Copy assignment operator: T& operator=(const T&) ：  Existing object replaced as a copy of another existing object
- Move constructor: T(T&&)
- Move assignment operator: T& operator=(T&&)

SMFs are automatically generated for you，
-  But if you’re managing pointers to allocated to memory, do it yourself



# Type Safety

The extent to which a function signature guarantees the behavior of a function.

## std::optional

std::optional is a template class which will either contain a value of type T or contain nothing (expressed as nullopt)

nullptr: an object that can be converted to a value of any pointer type
nullopt: an object that can be converted to a value of any optional type


# RAII (Resource Acquisition Is Initialization)

## Smart Pointers