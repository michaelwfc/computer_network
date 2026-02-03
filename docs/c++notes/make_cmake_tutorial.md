

# References
[csdiy](https://csdiy.wiki/%E5%BF%85%E5%AD%A6%E5%B7%A5%E5%85%B7/CMake/)
[how-to-write-makefile](https://seisman.github.io/how-to-write-makefile/overview.html)
[CMake Tutorial](https://cmake.org/cmake/help/latest/guide/tutorial/index.html)
[IPADS新人培训第二讲：CMake](https://www.bilibili.com/video/BV14h41187FZ/?vd_source=b3d4057adb36b9b243dc8d7a6fc41295)

# Make

## makefile的规则

```makefile
target ... : prerequisites ...
    recipe
    ...
    ...
```

prerequisites中如果有一个以上的文件比target文件要新的话，recipe所定义的命令就会被执行。
- target 
可以是一个object file（目标文件），也可以是一个可执行文件，还可以是一个标签（label）。对于标签这种特性，在后续的“伪目标”章节中会有叙述。
- prerequisites
生成该target所依赖的文件和/或target。
- recipe
该target要执行的命令（任意的shell命令）。

# CMake

CMake 是类似于 GNU make 的跨平台自动软件构建工具，使用 CMakeLists.txt 定义构建规则，相比于 make 它提供了更多的功能，在各种软件构建上广泛使用。强烈建议学习使用 GNU Make 和熟悉 Makefile 后再学习 CMake。

## CMakeLists.txt 文件

CMakeLists.txt 是 CMake 的配置文件，用于定义项目的构建规则、依赖关系、编译选项等。
每个 CMake 项目通常都有一个或多个 CMakeLists.txt 文件。



## How to To use CMAKE

### method 1:

1. You need to have a CMakeLists.txt file in your project’s root directory
2. Make a build folder (mkdir build) within your project!
3. Go into the build folder (cd build)
4. Run cmake ..
    a. This command runs cmake using the CMakeLists.txt in your project’s root folder!
    b. This generates a Makefile
5. Run make
6. Execute your program using ./main as usual

### method 2:

cmake -B build      # 生成构建目录
cmake --build build # 执行构建
./build/main        # 运行 main 程序
