cmake_minimum_required(VERSION 3.10)

project(cs106L)

set(CMAKE_CXX_STANDARD 17)

# 添加 utils 子目录
# add_subdirectory(...) 不能直接添加一个不在当前项目目录下的外部目录，除非你明确指定一个构建输出目录（即 binary dir）。
# SOURCE_DIR 是你 utils 库的源码路径。
# BINARY_DIR 是该库的构建输出目录（放在主项目的 build/ 目录里或其他地方均可）。
# CMake 这样做是为了避免污染当前项目的 build 目录。
add_subdirectory(/mnt/e/projects/computer_network/libs/utils  ${CMAKE_BINARY_DIR}/utils_build)

# 添加 main.cpp
add_executable(main main.cpp)

# 链接 utils 库
target_link_libraries(main utils)

# 添加包含目录，确保能找 Vector.tpp
# target_include_directories(main PRIVATE /mnt/e/projects/computer_network/libs/utils/src)
