# C++ dev env on windows
## C++ on WSL

- https://code.visualstudio.com/docs/cpp/config-wsl
- https://code.visualstudio.com/docs/remote/wsl
  
### WSL Ubuntu env Installation

 - install wsl and linux distribution
 - install vscode
 - install vscode for local wsl

```bash
wsl -l -v 
# connect to wsl terminal of Ubuntu-22.04
wsl -d Ubuntu-22.04 
lsb_release -a

g++ --version
whereis g++
gdb --version
whereis gdb
```

## Compilers

C/C++ extension does not include a C++ compiler. So, you will need to install one or use which is already installed on your computer.

- WSL(Linux): gcc + GDB

## C++ IDE 
- IDE : [vscode](https://code.visualstudio.com/docs/languages/cpp)
- vscode extension: Install recommended C/C++ extension in VSCode and reload
- https://code.visualstudio.com/docs/cpp/cpp-debug
- https://code.visualstudio.com/docs/cpp/launch-json-reference

### WSL Vscode settings

- c_cpp_properties.json (compiler path and IntelliSense settings)
- tasks.json (build instructions)
- launch.json (debugger settings)

## Hello World

```cpp
// This directive includes the standard input-output stream library, which allows us to use std::cout for printing to the console.
#include <iostream>

int hello_cpp() {
    std::cout << "Hello, World!" << std::endl;
    return 0;
}
```

```bash
# Compile and run
 # g++ is the compiler, outputs binary to main
g++ main.cpp –o main
# This actually runs our program 
./main		

g++ C++tutorials/STL/vector.cpp lib/include/printUtils.cpp -Ilib/include -o vector
./vector
```

### Why compile over interpret?

It allows us to generate more efficient machine code!
- Interpreters only see one small part of code at a time
- Compilers see everything
However, compilation takes time!

