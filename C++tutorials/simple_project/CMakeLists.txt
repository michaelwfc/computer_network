cmake_minimum_required(VERSION 3.0)
project(SimpleProject)

set(CMAKE_CXX_STANDARD 17)

include_directories(${PROJECT_SOURCE_DIR}/lib)

add_subdirectory(lib)
add_subdirectory(app)