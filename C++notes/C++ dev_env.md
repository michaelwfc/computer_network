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

## C++ dev tools

- IDE : [vscode](https://code.visualstudio.com/docs/languages/cpp)
- vscode extension: Install recommended C/C++ extension in VSCode and reload
- compiler : gcc/MS Visual C++ /clang

## Compilers
C/C++ extension does not include a C++ compiler. So, you will need to install one or use which is already installed on your computer.

- WSL(Linux): gcc + GDB


## WSL Vscode settings
- c_cpp_properties.json (compiler path and IntelliSense settings)
- tasks.json (build instructions)
- launch.json (debugger settings)


## References

- https://code.visualstudio.com/docs/cpp/cpp-debug
- https://code.visualstudio.com/docs/cpp/launch-json-reference

