

[csdiy](https://csdiy.wiki/%E5%BF%85%E5%AD%A6%E5%B7%A5%E5%85%B7/CMake/)

[IPADS新人培训第二讲：CMake](https://www.bilibili.com/video/BV14h41187FZ/?vd_source=b3d4057adb36b9b243dc8d7a6fc41295)

# CMakeLists.txt 文件

CMakeLists.txt 是 CMake 的配置文件，用于定义项目的构建规则、依赖关系、编译选项等。
每个 CMake 项目通常都有一个或多个 CMakeLists.txt 文件。



# How to To use CMAKE

## method 1:

1. You need to have a CMakeLists.txt file in your project’s root directory
2. Make a build folder (mkdir build) within your project!
3. Go into the build folder (cd build)
4. Run cmake ..
    a. This command runs cmake using the CMakeLists.txt in your project’s root folder!
    b. This generates a Makefile
5. Run make
6. Execute your program using ./main as usual

## method 2:

cmake -B build      # 生成构建目录
cmake --build build # 执行构建
./build/main        # 运行 main 程序
