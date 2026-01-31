

# Referece
- [Setting up your CS144 VM](https://web.archive.org/web/20220608085129/https://web.stanford.edu/class/cs144/vm_howto/)

# Coding

## suggestions

- Before submitting each assignment, please run `cmake --build build --target format` (to standardize the formatting), and make sure to remove any dummy code or comments that were included with the starter code.
- Also (optionally) run `cmake --build build --target tidy` to receive suggestions for improvements related to C++ programming practices.

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
# No LSB modules are available.
# Distributor ID: Ubuntu
# Description:    Ubuntu 22.04.5 LTS
# Release:        22.04
# Codename:       jammy

g++ --version
# michael@DESKTOP-2KLOSPO:/mnt/e/projects/computer_network$ g++ --version
# g++ (Ubuntu 11.4.0-1ubuntu1~22.04) 11.4.0
# Copyright (C) 2021 Free Software Foundation, Inc.
# This is free software; see the source for copying conditions.  There is NO
# warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

whereis g++
# michael@DESKTOP-2KLOSPO:/mnt/e/projects/computer_network$ whereis g++
# g++: /usr/bin/g++ /usr/share/man/man1/g++.1.gz

gdb --version
# michael@DESKTOP-2KLOSPO:/mnt/e/projects/computer_network$ gdb --version
# GNU gdb (Ubuntu 12.1-0ubuntu1~22.04.3) 12.1
# Copyright (C) 2022 Free Software Foundation, Inc.
# License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>
# This is free software: you are free to change and redistribute it.
# There is NO WARRANTY, to the extent permitted by law.

whereis gdb
# michael@DESKTOP-2KLOSPO:/mnt/e/projects/computer_network$ whereis gdb
# gdb: /usr/bin/gdb /etc/gdb /usr/include/gdb /usr/share/gdb /usr/share/man/man1/gdb.1.gz
```


Here's a quick overview of the most popular Ubuntu LTS versions, the default g++ (GCC) version they ship with, and the C++ standard they support by default:

Ubuntu Version	Default g++ Version	Default C++ Standard	Supported C++ Standards
Ubuntu 24.04 LTS	g++ 13	C++17	C++98, C++03, C++11, C++14, C++17, C++20, C++23 (partial)
Ubuntu 22.04 LTS	g++ 11	C++17	C++98, C++03, C++11, C++14, C++17, C++20 (partial), C++23 (some via PPA)
Ubuntu 20.04 LTS	g++ 9	C++14	C++98, C++03, C++11, C++14, C++17, C++20 (partial)
Ubuntu 18.04 LTS	g++ 7	C++14	C++98, C++03, C++11, C++14, C++17 (partial)
Ubuntu 16.04 LTS	g++ 5	C++11	C++98, C++03, C++11, C++14 (partial)

Here's the markdown table conversion:

## Ubuntu LTS Versions and GCC Support

| Ubuntu Version | Default g++ Version | Default C++ Standard | Supported C++ Standards |
|----------------|---------------------|----------------------|-------------------------|
| Ubuntu 24.04 LTS | g++ 13 | C++17 | C++98, C++03, C++11, C++14, C++17, C++20, C++23 (partial) |
| Ubuntu 22.04 LTS | g++ 11 | C++17 | C++98, C++03, C++11, C++14, C++17, C++20 (partial), C++23 (some via PPA) |
| Ubuntu 20.04 LTS | g++ 9 | C++14 | C++98, C++03, C++11, C++14, C++17, C++20 (partial) |
| Ubuntu 18.04 LTS | g++ 7 | C++14 | C++98, C++03, C++11, C++14, C++17 (partial) |
| Ubuntu 16.04 LTS | g++ 5 | C++11 | C++98, C++03, C++11, C++14 (partial) |

## Compilers

C/C++ extension does not include a C++ compiler. So, you will need to install one or use which is already installed on your computer.

- WSL(Linux): gcc + GDB


### Why compile over interpret?

It allows us to generate more efficient machine code!
- Interpreters only see one small part of code at a time
- Compilers see everything
However, compilation takes time!



## Upgrade GCC 11 to 12 or 13+

Issue:
In C++20, std::string is allowed in constexpr — but only if the standard library fully implements that feature. GCC only fully supports constexpr std::string in GCC 12 and later.


Because Ubuntu 22.04 only comes with GCC 11 by default, and the ubuntu-toolchain-r/test PPA allows you to install and use newer compilers.

Install a newer GCC version:

```bash
# This adds a PPA (Personal Package Archive) to your system's APT sources.
# The ppa:ubuntu-toolchain-r/test is a well-known official PPA that provides newer versions of GCC, G++, etc. for Ubuntu.
# It enables you to install GCC 12, 13, 14, etc., even if your Ubuntu release only ships with GCC 11.

sudo add-apt-repository ppa:ubuntu-toolchain-r/test
sudo apt update
sudo apt install g++-13

# Then switch to it:
# It registers a new version of g++ (version 13) with the update-alternatives system, so you can easily switch between multiple installed versions of g++ on your system.
# /usr/bin/g++	The symbolic link to manage (the "master link").
# g++	The name of the group (what you’ll use with update-alternatives --config).
# /usr/bin/g++-13	The actual path to the specific g++ version you want to register (in this case, version 13).
# 100	The priority. Higher numbers = higher preference when auto-selecting the default.
sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-13 100

ll /usr/bin/ |grep g++
# lrwxrwxrwx  1 root root          21 Jul 29 17:59 g++ -> /etc/alternatives/g++*
# lrwxrwxrwx  1 root root          23 May 13  2023 g++-11 -> x86_64-linux-gnu-g++-11*
# lrwxrwxrwx  1 root root          23 Jul 11  2023 g++-13 -> x86_64-linux-gnu-g++-13*
# lrwxrwxrwx  1 root root           6 Aug  5  2021 x86_64-linux-gnu-g++ -> g++-11*
# -rwxr-xr-x  1 root root      932680 May 13  2023 x86_64-linux-gnu-g++-11*
# -rwxr-xr-x  1 root root     1018800 Jul 11  2023 x86_64-linux-gnu-g++-13*



# get back

sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-11 50
sudo update-alternatives --config g++
```

# C++ IDE ：WSL Vscode settings

## Reference

- IDE : [vscode](https://code.visualstudio.com/docs/languages/cpp)
- vscode extension: Install recommended C/C++ extension in VSCode and reload
- https://code.visualstudio.com/docs/cpp/cpp-debug
- https://code.visualstudio.com/docs/cpp/launch-json-reference

## WSL Vscode settings

- c_cpp_properties.json (compiler path and IntelliSense settings)
- tasks.json (build instructions)
- launch.json (debugger settings)



# Hello World

## Source code
```cpp
// This directive includes the standard input-output stream library, which allows us to use std::cout for printing to the console.
#include <iostream>

int hello_cpp() {
    std::cout << "Hello, World!" << std::endl;
    return 0;
}
```

## Compile and run

```bash
# Compile and run
 # g++ is the compiler, outputs binary to main
g++ main.cpp –o main
# This actually runs our program 
./main		

g++ C++tutorials/STL/vector.cpp lib/include/printUtils.cpp -Ilib/include -o vector
./vector
```
